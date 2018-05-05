#pragma once

#include <gl/glew.h>

namespace glutils {

	class VertexBufferObject {
	private:
		GLuint vboID;
	public:
		VertexBufferObject();
		~VertexBufferObject();
	};
}