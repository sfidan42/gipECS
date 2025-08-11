//
// Created by sadettin on 8/9/25.
//

#ifndef COMPONENT_PRIMITIVES_H
#define COMPONENT_PRIMITIVES_H
#include <glm/glm.hpp>

namespace ecs {

	template<int dim>
	struct Position {
		union {
			struct {
				float x;
				float y;
				float z;
			};
			glm::vec<dim, float> loc;
		};
	};

	template<int dim>
	struct Velocity {
		union {
			struct {
				float vx;
				float vy;
				float vz;
			};
			glm::vec<dim, float> vec;
		};
	};

	template<int dim>
	struct Rotation {
		float degrees;
		union {
			struct {
				float pitch;
				float yaw;
				float roll;
			};
			glm::vec<dim, float> rot;
		};
	};

	struct Direction {
		union {
			struct {
				float i;
				float j;
				float k;
			};
			glm::vec3 dir;
		};
	};
}

#endif // COMPONENT_PRIMITIVES
