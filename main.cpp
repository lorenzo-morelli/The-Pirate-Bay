// This has been adapted from the Vulkan tutorial

#include "Starter.hpp"
#include "TextMaker.hpp"
#include "PerlinNoise.hpp"

#define INSTANCE_MAX 5000

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
    alignas(16) vec4 color[INSTANCE_MAX];
} positionsBuffer;

class Main;

class Main : public BaseProject {
protected:
    // Here you list all the Vulkan objects you need:

    // Descriptor Layouts [what will be passed to the shaders]
    DescriptorSetLayout DSLIsland{}, DSLSpawn{}, DSLSea{}, DSLSky{};

    // Pipelines [Shader couples]
    VertexDescriptor VD, VDSky;
    Pipeline pipelineIsland, pipelineSpawn, pipelineSea, pipelineSky;

    // Models, textures and Descriptors (values assigned to the uniforms)
    Model<Vertex> island, sea, spawn, sun;
    Model<VertexUV> sky;
    DescriptorSet DSIsland, DSSea, DSSpawn, DSSky;
    Texture texSkyDay, texSkyNight;

    TextMaker txt;

    float size = 0.025f;
    int instances = 0;
    bool spot = 0;

    // Other application parameters
    int width = 800;
    int height = 600;
    float Ar{};
    mat4 ViewPrj{};
    vec3 Pos = vec3(10.0f, 0.0f, 10.0f);
    vec3 cameraPos{};
    float Yaw = radians(0.0f);
    float Pitch = radians(0.0f);
    float Roll = radians(0.0f);

    // Here you set the main application parameters
    void setWindowParameters() override {
        // window size, titile and initial background
        windowWidth = width;
        windowHeight = height;
        windowTitle = "The Pirate Bay ☠";
        windowResizable = GLFW_TRUE;
        initialBackgroundColor = {180.0f / 255.0f, 255.0f / 255.0f, 255.0 / 255.0f, 1.0f};

        // Descriptor pool sizes
        uniformBlocksInPool = 200;
        texturesInPool = 200;
        setsInPool = 200;

        Ar = 4.0f / 3.0f;
    }

    // What to do when the window changes size
    void onWindowResize(int w, int h) override {
        width = w;
        height = h;
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
                {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
                {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
        });

        DSLSpawn.init(this, {
                {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
                {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
                {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT}
        });

        DSLSea.init(this, {
                {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
                {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
        });

        DSLSky.init(this, {
                {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
                {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
                {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},
                {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
        });

        // Vertex descriptors
        VD.init(
                this,
                {{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}},
                {{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos),  sizeof(vec3), POSITION},
                 {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, norm), sizeof(vec3), NORMAL}}
        );

        VDSky.init(
                this,
                {{0, sizeof(VertexUV), VK_VERTEX_INPUT_RATE_VERTEX}},
                {{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexUV, pos),  sizeof(vec3), POSITION},
                 {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexUV, norm), sizeof(vec3), NORMAL},
                 {0, 2, VK_FORMAT_R32G32_SFLOAT,    offsetof(VertexUV, UV),  sizeof(vec2), UV}}
        );

        // Pipelines [Shader couples]
        // The last array, is a vector of pointer to the layouts of the sets that will
        // be used in this pipeline. The first element will be set 0, and so on...
        pipelineIsland.init(this, &VD, "shaders/PhongVert.spv", "shaders/ToonFrag.spv", {&DSLIsland});
        pipelineSpawn.init(this, &VD, "shaders/PhongCubesVert.spv", "shaders/ToonCubeFrag.spv", {&DSLSpawn});
        pipelineSea.init(this, &VD, "shaders/SeaVert.spv", "shaders/SeaFrag.spv", {&DSLSea});
        pipelineSky.init(this, &VDSky, "shaders/SkyVert.spv", "shaders/SkyFrag.spv", {&DSLSky});

        pipelineIsland.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
        pipelineSpawn.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
        pipelineSea.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
        pipelineSky.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);

        // Models, textures and Descriptors (values assigned to the uniforms)
        createGrid(island.vertices, island.indices);
        createPlane(sea.vertices,sea.indices,-100.0f,-100.0f,1000.0f);
        createCubeMesh(spawn.vertices, spawn.indices, 0, 0, 0, 0);
        createCubeMesh(sun.vertices, sun.indices, 0, 0, 0, 0);
        createSphereMesh(sky.vertices, sky.indices);

        island.initMesh(this, &VD);
        spawn.initMesh(this, &VD);
        sea.initMesh(this,&VD);
        sun.initMesh(this, &VD);
        sky.initMesh(this, &VDSky);

        texSkyDay.init(this, "textures/skyDay.jpg");
        texSkyNight.init(this, "textures/skyNight.jpg");

        txt.init(this, &demoText, width, height);
    }

    // Here you create your pipelines and Descriptor Sets!
    void pipelinesAndDescriptorSetsInit() override {
        // This creates a new pipeline (with the current surface), using its shaders
        pipelineIsland.create();
        pipelineSpawn.create();
        pipelineSea.create();
        pipelineSky.create();

        DSIsland.init(this, &DSLIsland, {
                {0, UNIFORM, sizeof(UniformBufferObject),       nullptr},
                {1, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
        });
        DSSpawn.init(this, &DSLSpawn, {
                {0, UNIFORM, sizeof(UniformBufferObject),       nullptr},
                {1, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
                {2, UNIFORM, sizeof(PositionsBuffer),           nullptr}
        });

        DSSea.init(this, &DSLSea, {
                {0, UNIFORM, sizeof(UniformBufferObject),       nullptr},
                {1, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
        });

        DSSky.init(this, &DSLSky, {
                {0, UNIFORM, sizeof(UniformBufferObject),       nullptr},
                {1, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
                {2, TEXTURE, 0,                   &texSkyDay},
                {3, TEXTURE, 0,                   &texSkyNight}
        });

        txt.pipelinesAndDescriptorSetsInit();
    }

    // Here you destroy your pipelines and Descriptor Sets!
    void pipelinesAndDescriptorSetsCleanup() override {
        pipelineIsland.cleanup();
        pipelineSpawn.cleanup();
        pipelineSea.cleanup();
        pipelineSky.cleanup();

        DSIsland.cleanup();
        DSSea.cleanup();
        DSSpawn.cleanup();
        DSSky.cleanup();

        txt.pipelinesAndDescriptorSetsCleanup();
    }

    // Here you destroy all the Models, Texture and Desc. Set Layouts you created!
    // You also have to destroy the pipelines
    void localCleanup() override {
        texSkyDay.cleanup();
        texSkyNight.cleanup();

        island.cleanup();
        sea.cleanup();
        spawn.cleanup();
        sun.cleanup();
        sky.cleanup();

        DSLIsland.cleanup();
        DSLSpawn.cleanup();
        DSLSea.cleanup();
        DSLSky.cleanup();

        pipelineIsland.destroy();
        pipelineSea.destroy();
        pipelineSpawn.destroy();
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

        sun.bind(commandBuffer);
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
        gubo.spot = spot;
        if (glfwGetKey(window, GLFW_KEY_N)) spot == 0 ? spot = 1 : spot = 0;

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) {
            cout << "nuovo cubbo!\n";
            spawnCube();
        }

        if (glfwGetKey(window, GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, GL_TRUE);

        float deltaT;
        vec3 m = vec3(0.0f), r = vec3(0.0f);
        bool fire = false;
        getSixAxis(deltaT, m, r, fire);
        gameLogic(deltaT, r);
        gamePhysics(deltaT, m);

        UniformBufferObject ubo{};

        ubo.mMat = scale(mat4(1), vec3(3));
        ubo.mvpMat = ViewPrj * ubo.mMat;
        ubo.nMat = inverse(transpose(ubo.mMat));

        switch ((int) spot) {
            case 0: {
                gubo.lightDir = normalize(vec3(0.0f, 0.0f, 0.0f));
                gubo.lightColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
                gubo.eyePos = cameraPos;
                break;
            }
            case 1: {
                float dang = Pitch + radians(15.0f);
                gubo.lightPos = Pos + vec3(0, 1, 0);
                gubo.lightDir = vec3(cos(dang) * sin(Yaw), sin(dang), cos(dang) * cos(Yaw));
                gubo.lightColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
                gubo.eyePos = cameraPos;
                break;
            }
            default:
                break;
        }

        static float L_time = 0.0f;
        L_time += deltaT;
        gubo.time = L_time;

        DSIsland.map((int) currentImage, &ubo, sizeof(ubo), 0);
        DSIsland.map((int) currentImage, &gubo, sizeof(gubo), 1);

        DSSea.map((int) currentImage, &ubo, sizeof(ubo), 0);
        DSSea.map((int) currentImage, &gubo, sizeof(gubo), 1);

        DSSpawn.map((int) currentImage, &ubo, sizeof(ubo), 0);
        DSSpawn.map((int) currentImage, &gubo, sizeof(gubo), 1);
        DSSpawn.map((int) currentImage, &positionsBuffer, sizeof(positionsBuffer), 2);

        DSSky.map((int) currentImage, &ubo, sizeof(ubo), 0);
        DSSky.map((int) currentImage, &gubo, sizeof(gubo), 1);
    }

//    void spawnIsland() {
//        int n = (int) sqrt(INSTANCE_ISLAND);
//        for (int i = 0; i < n; i++) {
//            for (int j = 0; j < n; j++) {
//                float noise = perlinNoise((float) i, (float) j);
//                vec4 pos = vec4((float) i * size, noise, (float) j * size, 0);
//                islandBuffer.pos[i * n + j] = pos;
//                cout << "i: " << i << " j: " << j << endl;
//            }
//        }
//    }

    void spawnCube() {
        float distance = 0.5f;
        float x = Pos.x / 3.0f - distance * sin(Yaw) * cos(Pitch);
        float y = (Pos.y + distance) / 3.0f - distance * sin(Pitch);
        float z = Pos.z / 3.0f - distance * cos(Yaw) * cos(Pitch);
        vec4 pos = vec4(x, y, z, 0);
        positionsBuffer.pos[instances % INSTANCE_MAX] = pos;
        instances++;
    }

    static float perlinNoise(float i, float j) {
        const siv::PerlinNoise::seed_type seed = 123456u;
        const siv::PerlinNoise perlin{seed};

        float x = i - 150.0f;
        float y = j - 150.0f;
        float distanceFromCenter = sqrt(x * x + y * y);

        // Define parameters for the Gaussian RBF
        float amplitude = 2.0f; // Amplitude of the RBF
        float sigmaSquared = 100.0f; // Variance of the RBF

        // Calculate a value using Perlin noise and Gaussian RBF with sigmoid smoothing
        auto perlinValue = (float) perlin.octave2D_01(i * 0.01f, j * 0.01f, 4);
        float normalizedDistanceFromCenter = distanceFromCenter / 150.0f; // Normalize distance to range [0,1]
        normalizedDistanceFromCenter *= normalizedDistanceFromCenter; // Square to increase effect towards center

        return amplitude * perlinValue * exp(-normalizedDistanceFromCenter * distanceFromCenter / sigmaSquared);
    }

    void gamePhysics(float deltaT, vec3 m) {
        const float MOVE_SPEED = 1.0f;
        static float jumpTime = 0.0f;

        // Position
        vec3 ux = rotate(mat4(1.0f), Yaw, vec3(0, 1, 0)) * vec4(1, 0, 0, 1);
        vec3 uz = rotate(mat4(1.0f), Yaw, vec3(0, 1, 0)) * vec4(0, 0, -1, 1);
        vec3 velocity = MOVE_SPEED * (m.x * ux + m.z * uz);

        // Apply acceleration due to gravity
        velocity.y -= 98.0f * deltaT;

        // Update position based on velocity
        Pos = Pos + velocity * deltaT;

        // Calculate the height of the ground using Perlin noise
        float groundLevel = perlinNoise((Pos.x / size) / 3.0f, (Pos.z / size) / 3.0f);
        groundLevel = groundLevel * 3.0f;

        // Check for collision with the ground
        if (Pos.y <= groundLevel) {
            Pos.y = groundLevel;
            // If the object is on the ground, reset its vertical velocity
            velocity.y = 0.0f;
        }

        // Jump with gravity
        if (glfwGetKey(window, GLFW_KEY_SPACE) && Pos.y <= groundLevel + 0.001f) {
            cout << "Jumping\n";
            // Only allow jumping if the object is very close to the ground (avoid double jumps)
            jumpTime = 5.0f;
        }

        if (jumpTime >= 0.0f) {
            velocity.y += jumpTime * jumpTime;
            jumpTime -= 0.5f;
        }

        // Update the position again after the jump
        Pos = Pos + velocity * deltaT;
    }

    void gameLogic(float deltaT, vec3 r) {
        const float FOVy = radians(80.0f);
        const float nearPlane = 0.1f;
        const float farPlane = 300.f;
        // Camera target height and distance
        const float camHeight = 0.5f;
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

    void createGrid(vector<Vertex> &vDef, vector<uint32_t> &vIdx) const;
    void createCubeMesh(vector<Vertex> &vDef, vector<uint32_t> &vIdx, int offset, float x, float y, float z) const;
    void createPlane(vector<Vertex> &vDef, vector<uint32_t> &vIdx, float originX, float originZ, float size) const;
    void createSphereMesh(vector<VertexUV> &vDef, vector<uint32_t> &vIdx);
};

#include "primGen.hpp"

int main() {
    Main app;
    try {
        app.run();
    } catch (const exception &e) {
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}