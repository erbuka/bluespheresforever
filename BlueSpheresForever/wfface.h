#pragma once

#include <vector>
#include <string>
#include "vecmath.h"


namespace WaveFront {

	class WFFace {
	public:
		std::string GetMaterial();
		std::string GetGroup();

		int GetVertexIndex(int vertex);
		int GetNormalIndex(int vertex);
		int GetTexCoordIndex(int vertex);

		int GetVerticesCount();

		bool ContainsVertexIndex(int vertex);

	private:

		WFFace();

		int verticesCount;

		std::string group;
		std::string material;

		std::vector<int> vertexIndices;
		std::vector<int> normalIndices;
		std::vector<int> texCoordIndices;

		void AddVertex(int iVertex);
		void AddVertex(int iVertex, int iTexCoords);
		void AddVertex(int iVertex, int iTexCoords, int iNormal);

		void SetMaterial(std::string m);
		void SetGroup(std::string g);

		friend class WFObject;
	};
}