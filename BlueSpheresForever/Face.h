#pragma once

#include <vector>
#include <string>
#include "vecmath.h"


namespace glutils {

	class Face {
	public:
		std::string getMaterial();
		std::string getGroup();

		int GetVertexIndex(int vertex);
		int GetNormalIndex(int vertex);
		int GetTexCoordIndex(int vertex);

		int GetVerticesCount();

	private:

		Face();

		int verticesCount;

		std::string group;
		std::string material;

		std::vector<int> vertexIndices;
		std::vector<int> normalIndices;
		std::vector<int> texCoordIndices;

		void addVertex(int iVertex);
		void addVertex(int iVertex, int iTexCoords);
		void addVertex(int iVertex, int iTexCoords, int iNormal);

		void setMaterial(std::string m);
		void setGroup(std::string g);

		friend class ObjectFile;
	};
}