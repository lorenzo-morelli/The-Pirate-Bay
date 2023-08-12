using namespace std;

void Main::createCubeMesh(vector<Vertex> &vDef, vector<uint32_t> &vIdx, int offset, float x, float y, float z) const {

    float startX = x*size;
    float endX = x*size + size;

    float startZ = z*size;
    float endZ = z*size + size;

    float startY = y - size/2;
    float endY = (size*2.0f)*y + size/2 ;

    //far
    vDef.push_back({{startX, startY, startZ}, {0.0f, 0.0f, -1.0f}});  // vertex 0 - Position and Normal
    vDef.push_back({{endX, startY, startZ}, {0.0f, 0.0f, -1.0f}});  // vertex 1 - Position and Normal
    vDef.push_back({{startX, endY, startZ}, {0.0f, 0.0f, -1.0f}});  // vertex 2 - Position and Normal
    vDef.push_back({{endX, endY, startZ}, {0.0f, 0.0f, -1.0f}});  // vertex 3 - Position and Normal

    vIdx.push_back(0+offset); vIdx.push_back(1+offset); vIdx.push_back(2+offset);  // First triangle
    vIdx.push_back(1+offset); vIdx.push_back(2+offset); vIdx.push_back(3+offset);  // Second triangle

    //near
    vDef.push_back({{startX, startY, endZ}, {0.0f, 0.0f, 1.0f}});  // vertex 4 - Position and Normal
    vDef.push_back({{endX, startY, endZ}, {0.0f, 0.0f, 1.0f}});  // vertex 5 - Position and Normal
    vDef.push_back({{startX, endY, endZ}, {0.0f, 0.0f, 1.0f}});  // vertex 6 - Position and Normal
    vDef.push_back({{endX, endY, endZ}, {0.0f, 0.0f, 1.0f}});  // vertex 7 - Position and Normal

    vIdx.push_back(4+offset); vIdx.push_back(5+offset); vIdx.push_back(6+offset);  // Third triangle
    vIdx.push_back(5+offset); vIdx.push_back(6+offset); vIdx.push_back(7+offset);  // Fourth triangle

    //bottom
    vDef.push_back({{startX, startY, startZ}, {0.0f, -1.0f, 0.0f}});  // vertex 8 - Position and Normal
    vDef.push_back({{endX, startY, startZ}, {0.0f, -1.0f, 0.0f}});  // vertex 9 - Position and Normal
    vDef.push_back({{startX, startY, endZ}, {0.0f, -1.0f, 0.0f}});  // vertex 10 - Position and Normal
    vDef.push_back({{endX, startY, endZ}, {0.0f, -1.0f, 0.0f}});  // vertex 11 - Position and Normal

    vIdx.push_back(8+offset); vIdx.push_back(9+offset); vIdx.push_back(10+offset);
    vIdx.push_back(10+offset); vIdx.push_back(9+offset); vIdx.push_back(11+offset);

    // top
    vDef.push_back({{startX, endY, startZ}, {0.0f, 1.0f, 0.0f}});  // vertex 12 - Position and Normal
    vDef.push_back({{endX, endY, startZ}, {0.0f, 1.0f, 0.0f}});  // vertex 13 - Position and Normal
    vDef.push_back({{startX, endY, endZ}, {0.0f, 1.0f, 0.0f}});  // vertex 14 - Position and Normal
    vDef.push_back({{endX, endY, endZ}, {0.0f, 1.0f, 0.0f}});  // vertex 15 - Position and Normal

    vIdx.push_back(12+offset); vIdx.push_back(13+offset); vIdx.push_back(14+offset);
    vIdx.push_back(14+offset); vIdx.push_back(13+offset); vIdx.push_back(15+offset);

    // right face
    vDef.push_back({{endX, startY, startZ}, {1.0f, 0.0f, 0.0f}});  // vertex 16 - Position and Normal
    vDef.push_back({{endX, endY, startZ}, {1.0f, 0.0f, 0.0f}});  // vertex 17 - Position and Normal
    vDef.push_back({{endX, startY, endZ}, {1.0f, 0.0f, 0.0f}});  // vertex 18 - Position and Normal
    vDef.push_back({{endX, endY, endZ}, {1.0f, 0.0f, 0.0f}});  // vertex 19 - Position and Normal

    vIdx.push_back(16+offset); vIdx.push_back(17+offset); vIdx.push_back(18+offset);
    vIdx.push_back(18+offset); vIdx.push_back(17+offset); vIdx.push_back(19+offset);

    // left face
    vDef.push_back({{startX, startY, startZ}, {-1.0f, 0.0f, 0.0f}});  // vertex 20 - Position and Normal
    vDef.push_back({{startX, endY, startZ}, {-1.0f, 0.0f, 0.0f}});  // vertex 21 - Position and Normal
    vDef.push_back({{startX, startY, endZ}, {-1.0f, 0.0f, 0.0f}});  // vertex 22 - Position and Normal
    vDef.push_back({{startX, endY, endZ}, {-1.0f, 0.0f, 0.0f}});  // vertex 23 - Position and Normal

    vIdx.push_back(20+offset); vIdx.push_back(21+offset); vIdx.push_back(22+offset);
    vIdx.push_back(22+offset); vIdx.push_back(21+offset); vIdx.push_back(23+offset);
}

void Main::createGrid(vector<Vertex> &vDef, vector<uint32_t> &vIdx) {
    int n = 300;
    for (int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            int offset = (int) vDef.size();
            float noise = Main::perlinNoise((float) i, (float) j);
            createCubeMesh(vDef, vIdx, offset, (float) i, noise, (float) j);
        }
    }
}


void Main::createPlane(vector<Vertex> &vDef, vector<uint32_t> &vIdx, float originX, float originZ, float size) const {
    std::vector<float> vPos;

    float startX = originX;
    float endX = originX + size;

    float startZ = originZ;
    float endZ = originZ + size;

    const int resX = 1000;
    const int resZ = 1000;
    const float halfSizeX = 100;
    const float halfSizeZ = 100;

    for(int i = 0; i <= resX; i++) {
        for(int j = 0; j <= resZ; j++) {
            float u = (float)i / float(resX);
            float v = (float)j / float(resZ);
            float x = (2 * u - 1.0) * halfSizeX;
            float z = (2 * v - 1.0) * halfSizeZ;

            vPos.push_back(x); vPos.push_back(0.0); vPos.push_back(z);	// vertex 0
            vPos.push_back(u); vPos.push_back(v);	//UV
            vPos.push_back(0.0); vPos.push_back(1.0); vPos.push_back(0.0);	// Norm

            if((i < resX) && (j < resZ)) {
                int r1 = j * (resX + 1);
                int r2 = (j + 1) * (resX + 1);
                vIdx.push_back(r1+i);   vIdx.push_back(r1+i+1); vIdx.push_back(r2+i);
                vIdx.push_back(r1+i+1); vIdx.push_back(r2+i);   vIdx.push_back(r2+i+1);
            }
        }
    }

    for(int i = 0; i < vPos.size(); i+=8) {
        Vertex vertex{};
        vertex.pos = {vPos[i], vPos[i+1], vPos[i+2]};
        vertex.norm = {vPos[i+5], vPos[i+6], vPos[i+7]};
        vDef.push_back(vertex);
    }
}