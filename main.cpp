// This has been adapted from the Vulkan tutorial

#include "Starter.hpp"
#include "TextMaker.hpp"
#include "PerlinNoise.hpp"
#include <cstdlib>


#define INSTANCE_MAX 5000
#define ISLAND_SIZE 300
#define ROCKS 100
#define PALMS 23
#define SIZE 0.05f

using namespace glm;
using namespace std;

vector<SingleText> demoText = {{1, {"+", "", "", ""}, 0, 0}};

// The uniform buffer object used in this example
struct UniformBufferObject {
    alignas(16) mat4 mvpMat;
    alignas(16) mat4 mMat;
    alignas(16) mat4 nMat;
};

struct GlobalUniformBufferObject {
    alignas(4) bool spot;
    alignas(16) vec3 lightPos;
    alignas(16) vec3 lightDir;
    alignas(16) vec4 lightColor;
    alignas(16) vec3 eyePos;
    alignas(4) float time;
};

struct Vertex {
    vec3 pos;
    vec3 norm;
};

struct VertexUV {
    vec3 pos;
    vec3 norm;
    vec2 UV;
};

struct PositionsBuffer {
    alignas(16) vec4 pos[INSTANCE_MAX];
    alignas(4) bool hasGravity[INSTANCE_MAX];
} positionsBuffer;

struct PositionRocks {
    alignas(16) vec4 pos[ROCKS];
} positionRocks;

struct PositionPalms {
    alignas(16) vec4 pos[PALMS];
} positionPalms;


vector<int> movingCubes;

class Main;

class Main : public BaseProject {
protected:
    // Here you list all the Vulkan objects you need:

    // Descriptor Layouts [what will be passed to the shaders]
    DescriptorSetLayout DSLIsland{}, DSLSpawn{},
            DSLSea{}, DSLSky{}, DSLSun{},
            DSLRocks{}, DSLFlag{}, DSLFlagPole{},
            DSLPalmTrunk{}, DSLPalmLeaf{};

    // Pipelines [Shader couples]
    VertexDescriptor VD, VDuv;
    Pipeline pipelineIsland, pipelineSpawn,
            pipelineRocks, pipelineSea, pipelineSky,
            pipelineSun, pipelineFlag, pipelineFlagPole,
            pipelinePalmTrunk, pipelinePalmLeaf;

    // Models, textures and Descriptors (values assigned to the uniforms)
    Model<Vertex> island, sea, spawn, sun, rock, flagPole;
    Model<VertexUV> sky, flag, palmTrunk, palmLeaf;
    DescriptorSet DSIsland, DSSea, DSSpawn, DSSky, DSSun, DSRock, DSFlag, DSFlagPole, DSPalmTrunk, DSPalmLeaf;
    Texture texSkyDay{}, texBlackFlag{};

    TextMaker txt;

    float size = SIZE;
    float heightMap[ISLAND_SIZE][ISLAND_SIZE]{};
    siv::PerlinNoise::seed_type seed{};
    int instances = 0;
    float center = ISLAND_SIZE * size / 2;

    // Other application parameters
    float Ar{};
    mat4 ViewPrj{};
    vec3 Pos = vec3(10.0f, 0.0f, 10.0f);
    vec3 cameraPos{};
    float camHeight = 0.5f;
    float Yaw = radians(0.0f);
    float Pitch = radians(0.0f);
    float Roll = radians(0.0f);

    // Here you set the main application parameters
    void setWindowParameters() override {
        // window size, titile and initial background
        windowWidth = 1422;
        windowHeight = 800;
        windowTitle = "The Pirate Bay ☠";
        windowResizable = GLFW_TRUE;
        initialBackgroundColor = {180.0f / 255.0f, 255.0f / 255.0f, 255.0 / 255.0f, 1.0f};

        // Descriptor pool sizes
        uniformBlocksInPool = 400;
        texturesInPool = 400;
        setsInPool = 400;

        Ar =  1422.0f / 800.0f;

        iconPath = "textures/Safeimagekit-resized-img.png";

    }

    // What to do when the window changes size
    void onWindowResize(int w, int h) override {
        Ar = (float) w / (float) h;
    }

    // Here you load and setup all your Vulkan Models and Textures.
    // Here you also create your Descriptor set layouts and load the shaders for the pipelines
    void localInit() override {
        // Descriptor Layouts [what will be passed to the shaders]
        DSLIsland.init(this, {
                // this array contains the binding:
                // first  element : the binding number
                // second element : the type of element (buffer or texture)
                // third  element : the pipeline stage where it will be used
                {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT}, // ubo
                {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS} // gubo
        });

        DSLSpawn.init(this, {
                {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT}, // UBO
                {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
                {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT}
        });

        DSLRocks.init(this, {
                {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT}, // UBO
                {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
                {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT}
        });

        DSLSea.init(this, {
                {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
                {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
        });

        DSLSky.init(this, {
                {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_VERTEX_BIT},
                {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_ALL_GRAPHICS},
                {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},
                {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
        });

        DSLFlag.init(this, {
                {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_VERTEX_BIT},
                {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_ALL_GRAPHICS},
                {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
        });

        DSLFlagPole.init(this, {
                {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
                {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
        });

        DSLPalmTrunk.init(this, {
                {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
                {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
                {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT}
        });

        DSLPalmLeaf.init(this, {
                {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
                {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
                {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT}
        });

        DSLSun.init(this, {
                {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
                {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
        });

        // Vertex descriptors
        VD.init(
                this,
                {{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}},
                {{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos),  sizeof(vec3), POSITION},
                 {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, norm), sizeof(vec3), NORMAL}}
        );

        VDuv.init(
                this,
                {{0, sizeof(VertexUV), VK_VERTEX_INPUT_RATE_VERTEX}},
                {{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexUV, pos),  sizeof(vec3), POSITION},
                 {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexUV, norm), sizeof(vec3), NORMAL},
                 {0, 2, VK_FORMAT_R32G32_SFLOAT,    offsetof(VertexUV, UV),   sizeof(vec2), UV}}
        );

        // Pipelines [Shader couples]
        // The last array, is a vector of pointer to the layouts of the sets that will
        // be used in this pipeline. The first element will be set 0, and so on...
        pipelineIsland.init(this, &VD, "shaders/IslandVert.spv", "shaders/IslandFrag.spv", {&DSLIsland});
        pipelineSpawn.init(this, &VD, "shaders/SpawnVert.spv", "shaders/SpawnFrag.spv", {&DSLSpawn});
        pipelineRocks.init(this, &VD, "shaders/RockVert.spv", "shaders/RockFrag.spv", {&DSLRocks});
        pipelineSea.init(this, &VD, "shaders/SeaVert.spv", "shaders/SeaFrag.spv", {&DSLSea});
        pipelineSky.init(this, &VDuv, "shaders/SkyVert.spv", "shaders/SkyFrag.spv", {&DSLSky});
        pipelineFlag.init(this, &VDuv, "shaders/FlagVert.spv", "shaders/FlagFrag.spv", {&DSLFlag});
        pipelineFlagPole.init(this, &VD, "shaders/FlagPoleVert.spv", "shaders/FlagPoleFrag.spv", {&DSLFlagPole});
        pipelinePalmTrunk.init(this, &VDuv, "shaders/PalmTrunkVert.spv", "shaders/PalmTrunkFrag.spv", {&DSLPalmTrunk});
        pipelinePalmLeaf.init(this, &VDuv, "shaders/PalmLeafVert.spv", "shaders/PalmLeafFrag.spv", {&DSLPalmLeaf});
        pipelineSun.init(this, &VD, "shaders/SunVert.spv", "shaders/SunFrag.spv", {&DSLSun});

        pipelineIsland.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
        pipelineSpawn.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
        pipelineRocks.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
        pipelineSea.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
        pipelineSky.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
        pipelineSun.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
        pipelineFlag.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
        pipelineFlagPole.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
        pipelinePalmTrunk.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
        pipelinePalmLeaf.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<siv::PerlinNoise::seed_type> dis;
        seed = dis(gen);

        // Models, textures and Descriptors (values assigned to the uniforms)
        createGrid(island.vertices, island.indices);
        createPlane(sea.vertices, sea.indices);
        createPlaneWithUV(flag.vertices, flag.indices);
        createCubeMesh(flagPole.vertices, flagPole.indices, 0, 0, 0, 0, 0, 1.0f);
        createCubeMesh(spawn.vertices, spawn.indices, 0, 0, 0, 0, 0, size);
        createCubeMesh(rock.vertices, rock.indices, 0, 0, 0, 0, 0, size);
        createCubeMesh(sun.vertices, sun.indices, 0, center, 3, center, 0, 10.0f);
        createSphereMesh(sky.vertices, sky.indices);

        island.initMesh(this, &VD);
        spawn.initMesh(this, &VD);
        rock.initMesh(this, &VD);
        sea.initMesh(this, &VD);
        sun.initMesh(this, &VD);
        sky.initMesh(this, &VDuv);
        flag.initMesh(this, &VDuv);
        flagPole.initMesh(this, &VD);

        palmTrunk.init(this, &VDuv, "models/PalmTrunk.obj", OBJ);
        palmLeaf.init(this, &VDuv, "models/PalmLeaf.obj", OBJ);

        texBlackFlag.init(this, "textures/blackFlag.png");
        texSkyDay.init(this, "textures/skyDay.jpg");

        for (auto &pos : positionRocks.pos) {
            int xRandom = rand() % (ISLAND_SIZE - 5); //rock size is 5 cubes
            int zRandom = rand() % (ISLAND_SIZE - 5);
            pos = vec4((float) xRandom * size, heightMap[xRandom][zRandom], (float) zRandom * size, 1.0f);
            //recompute heightMap
            float heightRock = heightMap[xRandom][zRandom] + 5.0f * size - 0.25f -
                               size; //5.0f*size is rockSize , 0.25 is rock offset
            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < 5; j++) {
                    if (heightMap[xRandom + i][zRandom + j] < heightRock)
                        heightMap[xRandom + i][zRandom + j] = heightRock;
                }
            }
        }

        normal_distribution<float> distribution(ISLAND_SIZE / 2.0f, ISLAND_SIZE / 9.0f);
        default_random_engine generator(seed);
        for (auto & pos : positionPalms.pos) {

            // Generate random coordinates within a smaller range around the center using Gaussian distribution
            int xRandom = (int) round(distribution(generator));
            int zRandom = (int) round(distribution(generator));
            // Ensure generated coordinates are within island bounds
            xRandom = std::max(2, std::min(ISLAND_SIZE - 1, xRandom)); //2 is size of Palm collider box
            zRandom = std::max(2, std::min(ISLAND_SIZE - 1, zRandom));

            pos = vec4((float) xRandom * size, heightMap[xRandom][zRandom], (float) zRandom * size, 1.0f);
            //recompute heightMap
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    heightMap[xRandom - i][zRandom - j] = FLT_MAX;
                }
            }
        }

        txt.init(this, &demoText);
    }

    // Here you create your pipelines and Descriptor Sets!
    void pipelinesAndDescriptorSetsInit() override {
        // This creates a new pipeline (with the current surface), using its shaders
        pipelineIsland.create();
        pipelineSpawn.create();
        pipelineRocks.create();
        pipelineSea.create();
        pipelineSky.create();
        pipelineFlag.create();
        pipelineFlagPole.create();
        pipelinePalmTrunk.create();
        pipelinePalmLeaf.create();
        pipelineSun.create();

        DSIsland.init(this, &DSLIsland, {
                {0, UNIFORM, sizeof(UniformBufferObject),       nullptr},
                {1, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
        });

        DSSpawn.init(this, &DSLSpawn, {
                {0, UNIFORM, sizeof(UniformBufferObject),       nullptr},
                {1, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
                {2, UNIFORM, sizeof(PositionsBuffer),           nullptr}
        });

        DSRock.init(this, &DSLRocks, {
                {0, UNIFORM, sizeof(UniformBufferObject),       nullptr},
                {1, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
                {2, UNIFORM, sizeof(PositionRocks),             nullptr}
        });

        DSSea.init(this, &DSLSea, {
                {0, UNIFORM, sizeof(UniformBufferObject),       nullptr},
                {1, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
        });

        DSSky.init(this, &DSLSky, {
                {0, UNIFORM, sizeof(UniformBufferObject),       nullptr},
                {1, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
                {2, TEXTURE, 0,                                 &texSkyDay}
        });

        DSFlag.init(this, &DSLFlag, {
                {0, UNIFORM, sizeof(UniformBufferObject),       nullptr},
                {1, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
                {2, TEXTURE, 0,                                 &texBlackFlag}
        });

        DSFlagPole.init(this, &DSLFlagPole, {
                {0, UNIFORM, sizeof(UniformBufferObject),       nullptr},
                {1, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
        });

        DSPalmTrunk.init(this, &DSLPalmTrunk, {
                {0, UNIFORM, sizeof(UniformBufferObject),       nullptr},
                {1, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
                {2, UNIFORM, sizeof(PositionRocks),             nullptr}
        });

        DSPalmLeaf.init(this, &DSLPalmLeaf, {
                {0, UNIFORM, sizeof(UniformBufferObject),       nullptr},
                {1, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
                {2, UNIFORM, sizeof(PositionRocks),             nullptr}
        });

        DSSun.init(this, &DSLSun, {
                {0, UNIFORM, sizeof(UniformBufferObject),       nullptr},
                {1, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
        });

        txt.pipelinesAndDescriptorSetsInit();
    }

    // Here you destroy your pipelines and Descriptor Sets!
    void pipelinesAndDescriptorSetsCleanup() override {
        pipelineIsland.cleanup();
        pipelineSpawn.cleanup();
        pipelineSea.cleanup();
        pipelineFlag.cleanup();
        pipelineFlagPole.cleanup();
        pipelinePalmTrunk.cleanup();
        pipelinePalmLeaf.cleanup();
        pipelineRocks.cleanup();
        pipelineSky.cleanup();
        pipelineSun.cleanup();

        DSIsland.cleanup();
        DSSea.cleanup();
        DSFlag.cleanup();
        DSFlagPole.cleanup();
        DSPalmTrunk.cleanup();
        DSPalmLeaf.cleanup();
        DSRock.cleanup();
        DSSpawn.cleanup();
        DSSky.cleanup();
        DSSun.cleanup();

        txt.pipelinesAndDescriptorSetsCleanup();
    }

    // Here you destroy all the Models, Texture and Desc. Set Layouts you created!
    // You also have to destroy the pipelines
    void localCleanup() override {
        texSkyDay.cleanup();
        texBlackFlag.cleanup();

        island.cleanup();
        sea.cleanup();
        flag.cleanup();
        flagPole.cleanup();
        palmTrunk.cleanup();
        palmLeaf.cleanup();
        spawn.cleanup();
        rock.cleanup();
        sun.cleanup();
        sky.cleanup();

        DSLIsland.cleanup();
        DSLSpawn.cleanup();
        DSLRocks.cleanup();
        DSLPalmTrunk.cleanup();
        DSLPalmLeaf.cleanup();
        DSLSea.cleanup();
        DSLSky.cleanup();
        DSLFlag.cleanup();
        DSLFlagPole.cleanup();
        DSLSun.cleanup();

        pipelineSpawn.destroy();
        pipelineFlag.destroy();
        pipelineFlagPole.destroy();
        pipelinePalmTrunk.destroy();
        pipelinePalmLeaf.destroy();
        pipelineRocks.destroy();
        pipelineIsland.destroy();
        pipelineSea.destroy();
        pipelineSky.destroy();

        txt.localCleanup();
    }

    // Here it is the creation of the command buffer:
    // You send to the GPU all the objects you want to draw,
    // with their buffers and textures
    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) override {
        pipelineIsland.bind(commandBuffer);
        island.bind(commandBuffer);
        DSIsland.bind(commandBuffer, pipelineIsland, currentImage);
        vkCmdDrawIndexed(
                commandBuffer,
                static_cast<uint32_t>(island.indices.size()),
                1,
                0,
                0,
                0
        );

        pipelineSea.bind(commandBuffer);
        sea.bind(commandBuffer);
        DSSea.bind(commandBuffer, pipelineSea, currentImage);
        vkCmdDrawIndexed(
                commandBuffer,
                static_cast<uint32_t>(sea.indices.size()),
                1,
                0,
                0,
                0
        );

        pipelineFlag.bind(commandBuffer);
        flag.bind(commandBuffer);
        DSFlag.bind(commandBuffer, pipelineFlag, currentImage);
        vkCmdDrawIndexed(
                commandBuffer,
                static_cast<uint32_t>(flag.indices.size()),
                1,
                0,
                0,
                0
        );

        pipelineFlagPole.bind(commandBuffer);
        flagPole.bind(commandBuffer);
        DSFlagPole.bind(commandBuffer, pipelineFlagPole, currentImage);
        vkCmdDrawIndexed(
                commandBuffer,
                static_cast<uint32_t>(flagPole.indices.size()),
                1,
                0,
                0,
                0
        );

        pipelinePalmTrunk.bind(commandBuffer);
        palmTrunk.bind(commandBuffer);
        DSPalmTrunk.bind(commandBuffer, pipelinePalmTrunk, currentImage);
        vkCmdDrawIndexed(
                commandBuffer,
                static_cast<uint32_t>(palmTrunk.indices.size()),
                PALMS,
                0,
                0,
                0
        );

        pipelinePalmLeaf.bind(commandBuffer);
        palmLeaf.bind(commandBuffer);
        DSPalmLeaf.bind(commandBuffer, pipelinePalmLeaf, currentImage);
        vkCmdDrawIndexed(
                commandBuffer,
                static_cast<uint32_t>(palmLeaf.indices.size()),
                PALMS,
                0,
                0,
                0
        );

        pipelineSun.bind(commandBuffer);
        sun.bind(commandBuffer);
        DSSun.bind(commandBuffer, pipelineSun, currentImage);
        vkCmdDrawIndexed(
                commandBuffer,
                static_cast<uint32_t>(sun.indices.size()),
                1,
                0,
                0,
                0
        );

        pipelineSpawn.bind(commandBuffer);
        spawn.bind(commandBuffer);
        DSSpawn.bind(commandBuffer, pipelineSpawn, currentImage);
        vkCmdDrawIndexed(
                commandBuffer,
                static_cast<uint32_t>(spawn.indices.size()),
                INSTANCE_MAX,
                0,
                0,
                0
        );

        pipelineRocks.bind(commandBuffer);
        rock.bind(commandBuffer);
        DSRock.bind(commandBuffer, pipelineRocks, currentImage);
        vkCmdDrawIndexed(
                commandBuffer,
                static_cast<uint32_t>(rock.indices.size()),
                ROCKS,
                0,
                0,
                0
        );

        pipelineSky.bind(commandBuffer);
        sky.bind(commandBuffer);
        DSSky.bind(commandBuffer, pipelineSky, currentImage);
        vkCmdDrawIndexed(
                commandBuffer,
                static_cast<uint32_t>(sky.indices.size()),
                1,
                0,
                0,
                0
        );

        txt.populateCommandBuffer(commandBuffer, currentImage, 0);
    }

    // Here is where you update the uniforms.
    // Very likely this will be where you will be writing the logic of your application.
    void updateUniformBuffer(uint32_t currentImage) override {
        GlobalUniformBufferObject gubo{};
        UniformBufferObject ubo{};

        if (glfwGetKey(window, GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, GL_TRUE);

        float deltaT;
        vec3 m = vec3(0.0f), r = vec3(0.0f);
        bool fire = false;
        getSixAxis(deltaT, m, r, fire);
        gameLogic(deltaT, r);
        gamePhysics(deltaT, m);
        spawnLogic(deltaT);

        ubo.mMat = mat4(1);
        ubo.mvpMat = ViewPrj;
        ubo.nMat = inverse(transpose(ubo.mMat));

        static float L_time = 0.0f;
        L_time += deltaT;
        gubo.time = L_time/10.0f;

        if (sin(gubo.time) > -0.1) gubo.spot = false;
        else gubo.spot = true;

        if (!gubo.spot) {
            gubo.lightDir = normalize(vec3(cos(gubo.time), sin(gubo.time), 0.0f));
            gubo.lightColor = (1.0f-sin(gubo.time))*vec4(1.0f, 0.5f, 0.0f, 1.0f);
            gubo.eyePos = cameraPos;
        } else {
            float dang = Pitch + radians(15.0f);
            gubo.lightPos = Pos + vec3(0, 1, 0);
            gubo.lightDir = vec3(cos(dang) * sin(Yaw), sin(dang), cos(dang) * cos(Yaw));
            gubo.lightColor = vec4(1.0f, 1.0f, 1.0f, 1.0f); // white
            gubo.eyePos = cameraPos;
        }

        DSIsland.map((int) currentImage, &ubo, sizeof(ubo), 0);
        DSIsland.map((int) currentImage, &gubo, sizeof(gubo), 1);

        DSSea.map((int) currentImage, &ubo, sizeof(ubo), 0);
        DSSea.map((int) currentImage, &gubo, sizeof(gubo), 1);

        DSSpawn.map((int) currentImage, &ubo, sizeof(ubo), 0);
        DSSpawn.map((int) currentImage, &gubo, sizeof(gubo), 1);
        DSSpawn.map((int) currentImage, &positionsBuffer, sizeof(positionsBuffer), 2);

        DSRock.map((int) currentImage, &ubo, sizeof(ubo), 0);
        DSRock.map((int) currentImage, &gubo, sizeof(gubo), 1);
        DSRock.map((int) currentImage, &positionRocks, sizeof(positionRocks), 2);

        DSSky.map((int) currentImage, &ubo, sizeof(ubo), 0);
        DSSky.map((int) currentImage, &gubo, sizeof(gubo), 1);

        DSFlag.map((int) currentImage, &ubo, sizeof(ubo), 0);
        DSFlag.map((int) currentImage, &gubo, sizeof(gubo), 1);

        DSFlagPole.map((int) currentImage, &ubo, sizeof(ubo), 0);
        DSFlagPole.map((int) currentImage, &gubo, sizeof(gubo), 1);

        DSPalmTrunk.map((int) currentImage, &ubo, sizeof(ubo), 0);
        DSPalmTrunk.map((int) currentImage, &gubo, sizeof(gubo), 1);
        DSPalmTrunk.map((int) currentImage, &positionPalms, sizeof(positionPalms), 2);

        DSPalmLeaf.map((int) currentImage, &ubo, sizeof(ubo), 0);
        DSPalmLeaf.map((int) currentImage, &gubo, sizeof(gubo), 1);
        DSPalmLeaf.map((int) currentImage, &positionPalms, sizeof(positionPalms), 2);

        DSSun.map((int) currentImage, &ubo, sizeof(ubo), 0);
        DSSun.map((int) currentImage, &gubo, sizeof(gubo), 1);
    }

    void spawnLogic(float deltaT) {
        static float spawnTime = 0.0f;
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) && spawnTime <= 0.0f) { // with gravity
            spawnCube(true);
            spawnTime = 1.0f;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) && spawnTime <= 0.0f) { // without gravity
            spawnCube(false);
            spawnTime = 1.0f;
        }
        spawnTime -= 0.1f;

        for (auto it = movingCubes.begin(); it != movingCubes.end();) {
            int j = *it; //access value pointed by iterator (the ID of the moving cube so to update the position)

            if (positionsBuffer.hasGravity[j]) {
                float level = heightMap[(int) (positionsBuffer.pos[j].x / size)][(int) (positionsBuffer.pos[j].z /
                                                                                        size)];

                if (positionsBuffer.pos[j].y <= level) {
                    // Remove cube from movingCubes since it touched the ground
                    positionsBuffer.pos[j].y = level;
                    it = movingCubes.erase(it); // Erase and get updated iterator pointing to next element
                } else {
                    positionsBuffer.pos[j].y -= 98.0f * deltaT * deltaT;
                    ++it; // Move iterator to next element
                }
            } else {
                ++it; // Move iterator to next element
            }
        }
    }

    void spawnCube(bool hasGravity) {
        float distance = 0.5f;
        float x = Pos.x - distance * sin(Yaw) * cos(Pitch);
        float y = Pos.y + distance - distance * sin(Pitch);
        float z = Pos.z - distance * cos(Yaw) * cos(Pitch);

        //compute heightMap (grid) coordiantes

        int gridX = (int) (x / size);
        int gridZ = (int) (z / size);

        //grid alignment
        x = (float) gridX * size;
        z = (float) gridZ * size;


        //increase terrain height by 1 cube (size)
        heightMap[gridX][gridZ] += size;

        vec4 pos = vec4(x, y, z, 0);
        int cubeID = instances % INSTANCE_MAX;
        positionsBuffer.pos[cubeID] = pos;
        positionsBuffer.hasGravity[cubeID] = hasGravity;
        movingCubes.push_back(cubeID); //add the new spawned cube to the vector of moving cubes
        instances++;
    }

    void gamePhysics(float deltaT, vec3 m) {
        static float speed = 1.0f;
        float maxSpeed = 2.0f;
        float minSpeed = 1.0f;
        static float jumpTime = 0.0f;
        float groundLevel;
        int gridX = (int) (Pos.x / size);
        int gridZ = (int) (Pos.z / size);
        if (gridX >= 0 && gridX < ISLAND_SIZE && gridZ >= 0 && gridZ < ISLAND_SIZE)
            groundLevel = perlinNoise(Pos.x,Pos.z);
        else
            groundLevel = 0.0f;

        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) && speed < maxSpeed) speed += 0.07f;
        else if (speed > minSpeed) speed -= 0.1f;

        // Position
        vec3 ux = rotate(mat4(1.0f), Yaw, vec3(0, 1, 0)) * vec4(1, 0, 0, 1);
        vec3 uz = rotate(mat4(1.0f), Yaw, vec3(0, 1, 0)) * vec4(0, 0, -1, 1);
        vec3 velocity = speed * (m.x * ux + m.z * uz);

        // Apply acceleration due to gravity
        velocity.y -= 98.0f * deltaT;

        // Update position based on velocity
        vec3 nextPos = Pos + velocity * deltaT;
        float nextGroundLevel = heightMap[(int) (nextPos.x / size)][(int) (nextPos.z / size)];
        float diffHeight = abs(nextGroundLevel - groundLevel);

        if(diffHeight < 0.3f) { //in this way can't go over high colliders (like palms)
            Pos = nextPos;
        }

        // Check for collision with the ground
        if (Pos.y <= groundLevel) {
            Pos.y = groundLevel;
            // If the object is on the ground, reset its vertical velocity
            velocity.y = 0.0f;
        }

        // Jump with gravity
        if (glfwGetKey(window, GLFW_KEY_SPACE) && Pos.y <= groundLevel + 0.001f) {
            // Only allow jumping if the object is very close to the ground (avoid double jumps)
            jumpTime = 3.0f;
        }

        if (jumpTime >= 0.0f) {
            velocity.y += jumpTime * jumpTime;
            Pos.y += velocity.y * deltaT;
            jumpTime -= 0.1f;
        }
    }

    void gameLogic(float deltaT, vec3 r) {
        const float FOVy = radians(80.0f);
        const float nearPlane = 0.01f;
        const float farPlane = 300.f;
        // Camera target height and distance
        const float camDist = 0.0001f;
        // Camera Pitch limits
        const float minPitch = radians(-60.0f);
        const float maxPitch = radians(60.0f);
        // Rotation and motion speed
        const float ROT_SPEED = radians(120.0f);

        ViewPrj = mat4(1);
        mat4 World = mat4(1);

        // Rotation
        Yaw = Yaw - ROT_SPEED * deltaT * r.y;
        Pitch = Pitch + ROT_SPEED * deltaT * r.x;
        Pitch = Pitch < minPitch ? minPitch :
                (Pitch > maxPitch ? maxPitch : Pitch);
        Roll = Roll - ROT_SPEED * deltaT * r.z;
        Roll = Roll < radians(-175.0f) ? radians(-175.0f) :
               (Roll > radians(175.0f) ? radians(175.0f) : Roll);

        // Final world matrix computaiton
        World = translate(mat4(1), Pos) * rotate(mat4(1.0f), Yaw, vec3(0, 1, 0));

        // Projection
        mat4 Prj = perspective(FOVy, Ar, nearPlane, farPlane);
        Prj[1][1] *= -1;

        // Target
        vec3 target = Pos + vec3(0.0f, camHeight, 0.0f);

        // Camera position, depending on Yaw parameter, but not character direction
        cameraPos = World * vec4(0.0f, camHeight + camDist * sin(Pitch), camDist * cos(Pitch), 1.0);
        // Damping of camera
        mat4 View = rotate(mat4(1.0f), -Roll, vec3(0, 0, 1)) *
                    lookAt(cameraPos, target, vec3(0, 1, 0));

        ViewPrj = Prj * View;
    }

    [[nodiscard]] float perlinNoise(float x, float y) const;
    void createGrid(vector<Vertex> &vDef, vector<uint32_t> &vIdx);
    static void createCubeMesh(vector<Vertex> &vDef, vector<uint32_t> &vIdx, int offset, float x, float y, float z, float height, float cubeSize);
    static void createPlane(vector<Vertex> &vDef, vector<uint32_t> &vIdx);
    static void createPlaneWithUV(vector<VertexUV> &vDef, vector<uint32_t> &vIdx);
    static void createSphereMesh(vector<VertexUV> &vDef, vector<uint32_t> &vIdx);

};

#include "primGen.hpp"
#include <mmsystem.h>

int main() {
    try {
        PlaySound(R"(C:\Users\morel\Desktop\Programs\Uni\music\ciao\song.wav)", nullptr, SND_ASYNC);
    } catch (...) {
        cout << "No music" << endl;
    }
    Main app;
    try {
        app.run();
    } catch (const exception &e) {
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}