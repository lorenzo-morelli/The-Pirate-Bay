using namespace std;

void createCubeMesh(std::vector<Vertex> &vDef, std::vector<uint32_t> &vIdx, int offset, float size, float gap, float x, float y, float z) {

    float startX = x*(size+gap);
    float endX = x*(size+gap) + size;

    float startZ = z*(size+gap);
    float endZ = z*(size+gap) + size;

    float startY = y - size/2;
    float endY = y + size/2 ;

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


void Main::createGrid(std::vector<Vertex> &vDef, std::vector<uint32_t> &vIdx) {
    int n = 500;
    for (int i = 0; i < n; i++){
        for( int j = 0; j < n; j++){
            int offset = (int)vDef.size();
            float noise = Main::perlinNoise(i, j);
            createCubeMesh(vDef,vIdx, offset, size, 0.0f, (float)i,noise,(float)j);
            if (i == 126 && j == 54) std::cout << "IL NOSTRO NOISE E:::::::::::::::::" << noise;
        }
    }
}


void Main::createFunctionMesh(std::vector<Vertex> &vDef, std::vector<uint32_t> &vIdx) {
    // The surface is defined by equation y = sin(x) * cos(z) with -3 <= x <= 3 and -3 <= z <= 3
    int res = 40;
    for (int i = 0; i < res; i++) {
        for (int j = 0; j < res; j++) {
            float x = -3.0f + 6.0f * i / (float) res - 1;
            float z = -3.0f + 6.0f * j / (float) res - 1;
            float y = sin(x) * cos(z);
            // derivative to calculate the normal
            float dx = cos(x) * cos(z);
            float dy = 1.0f;
            float dz = -sin(z) * sin(x);
            float len = sqrt(dx * dx + dy * dy + dz * dz);
            dx /= len;
            dy /= len;
            dz /= len;
            vDef.push_back({{x, y, z}, {dx, dy, dz}});
        }
    }
    for (int i = 0; i < res - 1; i++) {
        for (int j = 0; j < res - 1; j++) {
            vIdx.push_back(i * res + j);
            vIdx.push_back(i * res + j + 1);
            vIdx.push_back((i + 1) * res + j);
            vIdx.push_back((i + 1) * res + j);
            vIdx.push_back(i * res + j + 1);
            vIdx.push_back((i + 1) * res + j + 1);
        }
    }

}

const float height = 2.0f;
void buildBase(std::vector<Vertex> &vDef, std::vector<uint32_t> &vIdx, int n, string name, int offset) {
    float y, y_norm;
    if (name == "bottom") {
        cout << "bottom" << endl;
        y = 0.0f;
        y_norm = -1.0f;
    } else {
        cout << "top" << endl;
        y = height;
        y_norm = 1.0f;
    }
    vDef.push_back({{0.0f, y, 0.0f}, {0.0f, y_norm, 0.0f}});  // center vertex
    for (int i = 1; i <= n; i++) {
        float x = cos(2 * M_PI * (i - 1) / n);
        float z = sin(2 * M_PI * (i - 1) / n);
        vDef.push_back({{x, y, z}, {0.0f, y_norm, 0.0f}});
        if (i == n) {
            vIdx.push_back(0 + offset); vIdx.push_back(i + offset); vIdx.push_back(1 + offset);
        } else {
            vIdx.push_back(0 + offset); vIdx.push_back(i + offset); vIdx.push_back(i + 1 + offset);
        }
    }
}

void buildSide(std::vector<Vertex> &vDef, std::vector<uint32_t> &vIdx, int n, int offset) {
    for (int i = 1; i <= 2 * n; i++) {
        float x1 = cos(2 * M_PI * (i - 1) / n);
        float z1 = sin(2 * M_PI * (i - 1) / n);
        float x2 = cos(2 * M_PI * i / n);
        float z2 = sin(2 * M_PI * i / n);
        vDef.push_back({{x1, 0.0f, z1}, {x1, 0.0f, z1}});  // bottom 1
        vDef.push_back({{x2, 0.0f, z2}, {x2, 0.0f, z2}});  // bottom 2
        vDef.push_back({{x1, height, z1}, {x1, 0.0f, z1}});  // top 1
        vDef.push_back({{x2, height, z2}, {x2, 0.0f, z2}});  // top 2
        vIdx.push_back(i * 2 + offset); vIdx.push_back(i * 2 + 1 + offset); vIdx.push_back(i * 2 + 2 + offset);
        vIdx.push_back(i * 2 + 1 + offset); vIdx.push_back(i * 2 + 3 + offset); vIdx.push_back(i * 2 + 2 + offset);

    }
}

void Main::createCylinderMesh(std::vector<Vertex> &vDef, std::vector<uint32_t> &vIdx) {
    // The procedure fills array vPos with the positions of the vertices and of the normal vectors of the mesh
    // The procedures also fill the array vIdx with the indices of the vertices of the triangles
    // The primitive built here is a cylinder, with radius 1, and height 2, centered in the origin.

    int res = 40;
    buildBase(vDef, vIdx, res, "bottom", (int) vDef.size());
    buildBase(vDef, vIdx, res, "top", (int) vDef.size());
    buildSide(vDef, vIdx, res, (int) vDef.size());
}