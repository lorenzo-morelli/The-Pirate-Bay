// This has been adapted from the Vulkan tutorial

#include "Starter.hpp"
#include "TextMaker.hpp"
#include "PerlinNoise.hpp"

#define INSTANCE_MAX 5000

using namespace glm;
using namespace std;

vector<SingleText> demoText = {
        {1, {"+",        "", "", ""}, 0, 0},
        {1, {"+",  "", "", ""}, 0, 0},
        {1, {"+", "", "", ""}, 0, 0}
};

// The uniform buffer object used in this example
struct UniformBufferObject {
    alignas(16) mat4 mvpMat;
    alignas(16) mat4 mMat;
    alignas(16) mat4 nMat;
};

struct GlobalUniformBufferObject {
    alignas(16) vec3 lightDir;
    alignas(16) vec4 lightColor;
    alignas(16) vec3 eyePos;
    alignas(4) float time;
};

struct GlobalUniformBufferObject3 {
    alignas(16) glm::vec3 lightPos;
    alignas(16) glm::vec3 lightDir;
    alignas(16) glm::vec4 lightColor;
    alignas(16) glm::vec3 eyePos;
};

struct Vertex {
    vec3 pos;
    vec3 norm;
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
    DescriptorSetLayout DSLIsland{}, DSLSpawn{}, DSL3{};

    // Pipelines [Shader couples]
    VertexDescriptor VD;
    Pipeline pipelineIsland, pipelineSpawn, P3;

    // Models, textures and Descriptors (values assigned to the uniforms)
    Model<Vertex> island, spawn, sun;
    DescriptorSet DSIsland, DSSpawn, DS3;


    TextMaker txt;

    float size = 0.025f;
    int instances = 0;

    float sunX = 0.0f;
    float sunY = 1.0f;

    // Other application parameters
    int width = 800;
    int height = 600;
    int currScene = 0;
    float Ar{};
    mat4 ViewPrj{};
    vec3 Pos = vec3(0.0f, 5.0f, 0.0f);
    vec3 cameraPos{};
    float Yaw = radians(0.0f);
    float Pitch = radians(0.0f);
    float Roll = radians(0.0f);

    // Here you set the main application parameters
    void setWindowParameters() override {
        // window size, titile and initial background
        windowWidth = width;
        windowHeight = height;
        windowTitle = "Fantastico Progetto di Morello e Piaggi";
        windowResizable = GLFW_TRUE;
        initialBackgroundColor = {180.0f / 255.0f, 255.0f / 255.0f, 255.0 / 255.0f, 1.0f};

        // Descriptor pool sizes
        uniformBlocksInPool = 7;
        texturesInPool = 4;
        setsInPool = 4;

        Ar = 4.0f / 3.0f;
    }

    // What to do when the window changes size
    void onWindowResize(int w, int h) override {
        width = w;
        height = h;
        Ar = (float) w / (float) h;
    }

    // Here you load and setup all your Vulkan Models and Texutures.
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
                // this array contains the binding:
                // first  element : the binding number
                // second element : the type of element (buffer or texture)
                // third  element : the pipeline stage where it will be used
                {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
                {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
                {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT}
        });

        DSL3.init(this, {
                // this array contains the binding:
                // first  element : the binding number
                // second element : the type of element (buffer or texture)
                // third  element : the pipeline stage where it will be used
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

        // Pipelines [Shader couples]
        // The last array, is a vector of pointer to the layouts of the sets that will
        // be used in this pipeline. The first element will be set 0, and so on...
        pipelineIsland.init(this, &VD, "shaders/PhongVert.spv", "shaders/ToonFrag.spv", {&DSLIsland});
        pipelineSpawn.init(this, &VD, "shaders/PhongCubesVert.spv", "shaders/ToonCubeFrag.spv", {&DSLSpawn});
        P3.init(this, &VD, "shaders/PhongSpotVert.spv", "shaders/ToonSpotFrag.spv", {&DSL3});

        pipelineIsland.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
        pipelineSpawn.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
        P3.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);

        // Models, textures and Descriptors (values assigned to the uniforms)
        createGrid(island.vertices, island.indices);
        createCubeMesh(spawn.vertices, spawn.indices, 0, 0, 0, 0);
        createCubeMesh(sun.vertices, sun.indices, 0, sunX, sunY, 0);
        island.initMesh(this, &VD);
        spawn.initMesh(this, &VD);
        sun.initMesh(this, &VD);
        txt.init(this, &demoText, width, height);
    }

    // Here you create your pipelines and Descriptor Sets!
    void pipelinesAndDescriptorSetsInit() override {
        // This creates a new pipeline (with the current surface), using its shaders
        pipelineIsland.create();
        pipelineSpawn.create();
        P3.create();

        DSIsland.init(this, &DSLIsland, {
                {0, UNIFORM, sizeof(UniformBufferObject),       nullptr},
                {1, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
        });
        DSSpawn.init(this, &DSLSpawn, {
                {0, UNIFORM, sizeof(UniformBufferObject),       nullptr},
                {1, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
                {2, UNIFORM, sizeof(PositionsBuffer),           nullptr}
        });
        DS3.init(this, &DSLIsland, {
                {0, UNIFORM, sizeof(UniformBufferObject),       nullptr},
                {1, UNIFORM, sizeof(GlobalUniformBufferObject3), nullptr}
        });
        txt.pipelinesAndDescriptorSetsInit();
    }

    // Here you destroy your pipelines and Descriptor Sets!
    void pipelinesAndDescriptorSetsCleanup() override {
        pipelineIsland.cleanup();
        pipelineSpawn.cleanup();
        P3.cleanup();
        DSIsland.cleanup();
        DSSpawn.cleanup();
        DS3.cleanup();
        txt.pipelinesAndDescriptorSetsCleanup();
    }

    // Here you destroy all the Models, Texture and Desc. Set Layouts you created!
    // You also have to destroy the pipelines
    void localCleanup() override {
        island.cleanup();
        spawn.cleanup();
        sun.cleanup();

        DSLIsland.cleanup();
        DSLSpawn.cleanup();
        DSL3.cleanup();

        pipelineIsland.destroy();
        pipelineSpawn.destroy();
        P3.destroy();

        txt.localCleanup();
    }

    // Here it is the creation of the command buffer:
    // You send to the GPU all the objects you want to draw,
    // with their buffers and textures
    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) override {
        switch (currScene) {
            case 0:
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
                break;
            case 1:
                P3.bind(commandBuffer);
                island.bind(commandBuffer);
                DS3.bind(commandBuffer, P3, currentImage);
                vkCmdDrawIndexed(
                        commandBuffer,
                        static_cast<uint32_t>(island.indices.size()),
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
                break;
            default:
                break;
        }
        txt.populateCommandBuffer(commandBuffer, currentImage, currScene);
    }

    // Here is where you update the uniforms.
    // Very likely this will be where you will be writing the logic of your application.
    void updateUniformBuffer(uint32_t currentImage) override {
        static bool debounce = false;
        static int button = 0;

        if (glfwGetKey(window, GLFW_KEY_N)) {
            if (!debounce) {
                debounce = true;
                button = GLFW_KEY_N;
                currScene = (currScene + 1) % 3;
                cout << "Scene : " << currScene << "\n";
                RebuildPipeline();
            }
        } else {
            if ((button == GLFW_KEY_N) && debounce) {
                debounce = false;
                button = 0;
            }
        }

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            cout << "nuovo cubbo!\n";
            spawnCube();
        }

        if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
        float deltaT;
        vec3 m = vec3(0.0f), r = vec3(0.0f);
        bool fire = false;
        getSixAxis(deltaT, m, r, fire);
        gameLogic(deltaT, r);
        gamePhysics(deltaT, m);

        UniformBufferObject ubo{};
        // Here is where you actually update your uniforms

        // updates global uniforms
        //TODO: ?????
        ubo.mMat = mat4(1);
        ubo.mvpMat = ViewPrj;
        ubo.nMat = inverse(transpose(ubo.mMat));


        ubo.mMat = scale(mat4(1), vec3(3));
        ubo.mvpMat = ViewPrj * ubo.mMat;
        ubo.nMat = inverse(transpose(ubo.mMat));

        switch(currScene) {
            case 0: {
                GlobalUniformBufferObject gubo{};
                gubo.lightDir = normalize(vec3(0.0f, 0.0f, 0.0f));
                gubo.lightColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
                gubo.eyePos = cameraPos;

                static float L_time = 0.0f;
                L_time += deltaT;
                gubo.time = L_time;
                sunMovement(L_time);
                DSIsland.map((int) currentImage, &ubo, sizeof(ubo), 0);
                DSIsland.map((int) currentImage, &gubo, sizeof(gubo), 1);
                DSSpawn.map((int) currentImage, &ubo, sizeof(ubo), 0);
                DSSpawn.map((int) currentImage, &gubo, sizeof(gubo), 1);
                DSSpawn.map((int) currentImage, &positionsBuffer, sizeof(positionsBuffer), 2);
                break;
            }
            case 1: {
                float dang = Pitch + glm::radians(15.0f);
                GlobalUniformBufferObject3 gubo3{};
                gubo3.lightPos = Pos + glm::vec3(0, 1, 0);
                gubo3.lightDir = glm::vec3(cos(dang) * sin(Yaw), sin(dang), cos(dang) * cos(Yaw));
                gubo3.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
                gubo3.eyePos = cameraPos;

                DS3.map(currentImage, &ubo, sizeof(ubo), 0);
                DS3.map(currentImage, &gubo3, sizeof(gubo3), 1);
                break;
            }
            default:
                break;
        }
    }

    void sunMovement(float time) {
        sunX = 3 * sin(time);
        sunY = 3 * cos(time);
    }

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

        // Calculate the distance from the center of the grid
        float x = (i - 250);
        float y = (j - 250);
        float distanceFromCenter = sqrt(x * x + y * y);

        // Define parameters for the Gaussian RBF
        float amplitude = 2.0f; // Amplitude of the RBF
        float sigmaSquared = 100.0f; // Variance of the RBF

        // Calculate a value using Perlin noise and Gaussian RBF with sigmoid smoothing
        auto perlinValue = (float) perlin.octave2D_01(i * 0.01f, j * 0.01f, 4);
        float normalizedDistanceFromCenter = distanceFromCenter / 250.0f; // Normalize distance to range [0,1]
        normalizedDistanceFromCenter *= normalizedDistanceFromCenter; // Square to increase effect towards center

        return amplitude * perlinValue * exp(-normalizedDistanceFromCenter * distanceFromCenter / sigmaSquared);

        //return 2.0f * (float) perlin.octave2D_01((i * 0.01), (j * 0.01), 4);

        //return 2.0f * sin(i*0.01f);
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
        const float farPlane = 100.f;
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

    void createGrid(vector<Vertex> &vDef, vector<uint32_t> &vIdx);

    void createCubeMesh(vector<Vertex> &vDef, vector<uint32_t> &vIdx, int offset, float x, float y, float z) const;
};

#include "primGen.hpp"


// This is the main: probably you do not need to touch this!
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