#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <cassert>
#include <memory>
#include <algorithm>

namespace ecs
{
	using entity = uint32_t;
	static constexpr entity null = 0;

	class EntityPool
	{
	public:
		entity	create()
		{
			if (free_.empty())
				return (++last_);
			entity e = free_.back();
			free_.pop_back();
			return e;
		}

		void	destroy(entity e)
		{
			if (e == null)
				return ;
			free_.push_back(e);
		}

		bool	isValid(entity e) const
		{
			return e != null && e <= last_ && !isFree(e);
		}

	private:

		bool	isFree(entity e) const
		{
			return std::find(free_.begin(), free_.end(), e) != free_.end();
		}

		entity				last_ = null;
		std::vector<entity>	free_;
	};


	template<typename T>
	class Storage
	{
		using index = size_t;
	public:
		bool	contains(entity e) const
		{
			auto it = sparse_.find(e);
			if (it == sparse_.end())
				return false;
			index idx = it->second;
			return (idx < dense_.size() && entities_[idx] == e);
		}

		template<class... Args>
		T&	emplace(entity e, Args&&... args)
		{
			assert(!contains(e));
			index idx = dense_.size();
			sparse_[e] = idx;
			entities_.push_back(e);
			dense_.emplace_back(std::forward<Args>(args)...);
			return dense_.back();
		}

		void	remove(entity e)
		{
			auto it = sparse_.find(e);
			if (it == sparse_.end())
				return ;
			index idx = it->second;
			index last = dense_.size() - 1;
			if (idx != last)
			{
				dense_[idx] = std::move(dense_[last]);
				entity moved = entities_[last];
				entities_[idx] = moved;
				sparse_[moved] = idx;
			}
			dense_.pop_back();
			entities_.pop_back();
			sparse_.erase(e);
		}

		T&	get(entity e)
		{
			auto it = sparse_.find(e);
			assert(it != sparse_.end());
			return dense_[it->second];
		}

		const T&	get(entity e) const
		{
			auto it = sparse_.find(e);
			assert(it != sparse_.end());
			return dense_[it->second];
		}

		const std::vector<entity>&	entities() const
		{
			return entities_;
		}

		std::vector<T>&	data()
		{
			return dense_;
		}

		const std::vector<T>&	data() const
		{
			return dense_;
		}

	private:
		std::unordered_map<entity, index>	sparse_;
		std::vector<entity>					entities_;
		std::vector<T>						dense_;
	};


	struct IStorage
	{
		virtual ~IStorage() = default;
	
		virtual void	remove(entity e) = 0;
		virtual bool	contains(entity e) const = 0;
	};


	template<typename T>
	struct StorageModel : IStorage
	{
		Storage<T>	storage;

		void	remove(entity e) override
		{
			storage.remove(e);
		}

		bool	contains(entity e) const override
		{
			return storage.contains(e);
		}
	};


	class Registry
	{
	public:
		entity	create()
		{
			return pool_.create();
		}

		void	destroy(entity e)
		{
			for (auto& kv : pools_)
				kv.second->remove(e);
			pool_.destroy(e);
		}

		template<class T, class... Args>
		T&	emplace(entity e, Args... args)
		{
			StorageModel<T>& sModel = assure<T>();
			return sModel.storage.emplace(e, std::forward<Args>(args)...);
		}

		template<class T>
		bool	all_of(entity e) const
		{
			std::type_index	idx(typeid(T));
			auto it = pools_.find(idx);
			if (it == pools_.end())
				return false;
			StorageModel<T>* sModel = static_cast<StorageModel<T>*>(it->second.get());
			return sModel->contains(e);
		}

		template<class T>
		T&	get(entity e)
		{
			std::type_index	idx(typeid(T));
			auto it = pools_.find(idx);
			assert(it != pools_.end());
			StorageModel<T>* sModel = static_cast<StorageModel<T>*>(it->second.get());
			return sModel->storage.get(e);
		}

		template<class... Ts>
		struct View
		{
			using index = size_t;

			Registry*	regPtr;

			struct iterator
			{
				const View*	vPtr;
				index		ePos;

				bool	operator!=(const iterator& other)
				{
					return ePos != other.ePos;
				}

				void	operator++()
				{
					ePos = vPtr->advanceToValid(ePos + 1);
				}

				entity	operator*() const
				{
					const auto& entities = vPtr->leadEntities();
					return entities[ePos];
				}
			};

			const std::vector<entity>&	leadEntities() const
			{
				using lead_t = typename std::tuple_element<0, std::tuple<Ts...>>::type;
				const StorageModel<lead_t>&	sModel = regPtr->assure<lead_t>();
				return sModel.storage.entities();
			}

			iterator	begin() const
			{
				return iterator
				{
					this,
					this->advanceToValid(0)
				};
			}

			iterator	end() const
			{
				return iterator
				{
					this,
					this->leadEntities().size()
				};
			}

			template<class T>
			T&	get(entity e)
			{
				return regPtr->get<T>(e);
			}

			index	advanceToValid(index start) const
			{
				const auto& entities = leadEntities();
				while (start < entities.size())
				{
					if (matches(entities[start]))
						return start;
					++start;
				}
				return start;
			}

			bool	matches(entity e) const
			{
				bool ok = true;
				using expand = int[];
				(void)expand{0, (ok = ok && regPtr->template all_of<Ts>(e), 0)...};
				return ok;
			}
		};

		template<class... Ts>
		View<Ts...>	view_all()
		{
			return View<Ts...>{this};
		}

	private:
		template<class T>
		StorageModel<T>&	assure() const
		{
			std::type_index	idx(typeid(T));
			auto it = pools_.find(idx);

			if (it == pools_.end())
			{
				auto ptr = std::unique_ptr<IStorage>(new StorageModel<T>());
				auto* raw = static_cast<StorageModel<T>*>(ptr.get());
				const_cast<Registry*>(this)->pools_.emplace(idx, std::move(ptr));
				return *raw;
			}
			StorageModel<T>*	sModel = static_cast<StorageModel<T>*>(it->second.get());
			return *sModel;
		}

		using PoolsType = std::unordered_map<std::type_index, std::unique_ptr<IStorage>>;

		mutable PoolsType	pools_;
		EntityPool			pool_;
	};
} // namespace entt2

//#include "entt2/EntityPool.hpp"
//#include "entt2/Storage.tpp"
//#include "entt2/StorageModel.tpp"
//#include "entt2/Registry.tpp"
//#include "entt2/Registry.View.tpp"
//#include "entt2/Registry.View.iterator.tpp"
