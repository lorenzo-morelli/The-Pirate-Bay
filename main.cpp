// This has been adapted from the Vulkan tutorial

#include "Starter.hpp"
#include "TextMaker.hpp"
#include "PerlinNoise.hpp"

using namespace glm;


std::vector<SingleText> demoText = {
        {1, {"Cube",     "", "", ""}, 0, 0},
        {1, {"Surface",  "", "", ""}, 0, 0},
        {1, {"Cylinder", "", "", ""}, 0, 0}
};

// The uniform buffer object used in this example
struct UniformBufferObject {
    alignas(16) mat4 mvpMat;
    alignas(16) mat4 mMat;
    alignas(16) mat4 nMat;
};

struct GlobalUniformBufferObject {
    alignas(16) vec3 selector;
    alignas(16) vec3 lightDir;
    alignas(16) vec4 lightColor;
    alignas(16) vec3 eyePos;
    alignas(4) float time;
};

struct Vertex {
    vec3 pos;
    vec3 norm;
};

class Main;

class Main : public BaseProject {
protected:
    // Here you list all the Vulkan objects you need:

    // Descriptor Layouts [what will be passed to the shaders]
    DescriptorSetLayout DSL1;

    // Pipelines [Shader couples]
    VertexDescriptor VD;
    Pipeline P1;

    // Models, textures and Descriptors (values assigned to the uniforms)
    Model<Vertex> M1, M2, M3;
    DescriptorSet DS1;

    TextMaker txt;

    // Other application parameters
    int currScene = 0;
    float Ar;
    mat4 ViewPrj;
    vec3 Pos = vec3(0.0f, 5.0f, 0.0f);
    vec3 cameraPos;
    float Yaw = radians(0.0f);
    float Pitch = radians(0.0f);
    float Roll = radians(0.0f);

    // Here you set the main application parameters
    void setWindowParameters() {
        // window size, titile and initial background
        windowWidth = 800;
        windowHeight = 600;
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
    void onWindowResize(int w, int h) {
        Ar = (float) w / (float) h;
    }

    // Here you load and setup all your Vulkan Models and Texutures.
    // Here you also create your Descriptor set layouts and load the shaders for the pipelines
    void localInit() {
        // Descriptor Layouts [what will be passed to the shaders]
        DSL1.init(this, {
                // this array contains the binding:
                // first  element : the binding number
                // second element : the type of element (buffer or texture)
                // third  element : the pipeline stage where it will be used
                {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
                {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
        });

        // Vertex descriptors
        VD.init(this, {
                {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}
        }, {
                        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos),
                                sizeof(vec3), POSITION},
                        {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, norm),
                                sizeof(vec3), NORMAL}
                });

        // Pipelines [Shader couples]
        // The last array, is a vector of pointer to the layouts of the sets that will
        // be used in this pipeline. The first element will be set 0, and so on..
        P1.init(this, &VD, "shaders/PhongVert.spv", "shaders/ToonFrag.spv", {&DSL1});
        P1.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL,
                               VK_CULL_MODE_NONE, false);

        // Models, textures and Descriptors (values assigned to the uniforms)
        createGrid(M1.vertices, M1.indices);
        M1.initMesh(this, &VD);

        createFunctionMesh(M2.vertices, M2.indices);
        createFunctionMesh(M2.vertices, M2.indices);
        M2.initMesh(this, &VD);

        createCylinderMesh(M3.vertices, M3.indices);
        M3.initMesh(this, &VD);

        txt.init(this, &demoText);
    }

    // Here you create your pipelines and Descriptor Sets!
    void pipelinesAndDescriptorSetsInit() {
        // This creates a new pipeline (with the current surface), using its shaders
        P1.create();

        DS1.init(this, &DSL1, {
                {0, UNIFORM, sizeof(UniformBufferObject),       nullptr},
                {1, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
        });
        txt.pipelinesAndDescriptorSetsInit();
    }

    // Here you destroy your pipelines and Descriptor Sets!
    void pipelinesAndDescriptorSetsCleanup() {
        P1.cleanup();

        DS1.cleanup();

        txt.pipelinesAndDescriptorSetsCleanup();
    }

    // Here you destroy all the Models, Texture and Desc. Set Layouts you created!
    // You also have to destroy the pipelines
    void localCleanup() {
        M1.cleanup();
        M2.cleanup();
        M3.cleanup();

        DSL1.cleanup();

        P1.destroy();

        txt.localCleanup();
    }

    // Here it is the creation of the command buffer:
    // You send to the GPU all the objects you want to draw,
    // with their buffers and textures
    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
        switch (currScene) {
            case 0:
                P1.bind(commandBuffer);
                M1.bind(commandBuffer);
                DS1.bind(commandBuffer, P1, currentImage);

                vkCmdDrawIndexed(commandBuffer,
                                 static_cast<uint32_t>(M1.indices.size()), 1, 0, 0, 0);
                break;

            case 1:
                P1.bind(commandBuffer);
                M2.bind(commandBuffer);
                DS1.bind(commandBuffer, P1, currentImage);

                vkCmdDrawIndexed(commandBuffer,
                                 static_cast<uint32_t>(M2.indices.size()), 1, 0, 0, 0);

                P1.bind(commandBuffer);
                M1.bind(commandBuffer);
                DS1.bind(commandBuffer, P1, currentImage);

                vkCmdDrawIndexed(commandBuffer,
                                 static_cast<uint32_t>(M1.indices.size()), 1, 0, 0, 0);

                break;
            case 2:
                P1.bind(commandBuffer);
                M3.bind(commandBuffer);
                DS1.bind(commandBuffer, P1, currentImage);

                vkCmdDrawIndexed(commandBuffer,
                                 static_cast<uint32_t>(M3.indices.size()), 1, 0, 0, 0);
                break;
        }

        txt.populateCommandBuffer(commandBuffer, currentImage, currScene);
    }

    // Here is where you update the uniforms.
    // Very likely this will be where you will be writing the logic of your application.
    void updateUniformBuffer(uint32_t currentImage) {
        static bool debounce = false;
        static int button = 0;

        if (glfwGetKey(window, GLFW_KEY_N)) {
            if (!debounce) {
                debounce = true;
                button = GLFW_KEY_N;
                currScene = (currScene + 1) % 3;
                std::cout << "Scene : " << currScene << "\n";
//				Pos = vec3(0,0,currScene == 0 ? 10 : 8);
                RebuildPipeline();
            }
        } else {
            if ((button == GLFW_KEY_N) && debounce) {
                debounce = false;
                button = 0;
            }
        }

        static bool showNormal = false;

        if (glfwGetKey(window, GLFW_KEY_SPACE)) {
            if (!debounce) {
                debounce = true;
                button = GLFW_KEY_SPACE;
                showNormal = !showNormal;
            }
        } else {
            if ((button == GLFW_KEY_SPACE) && debounce) {
                debounce = false;
                button = 0;
            }
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
        ubo.mMat = mat4(1);
        ubo.mvpMat = ViewPrj;
        ubo.nMat = inverse(transpose(ubo.mMat));

        float dang = Pitch + radians(15.0f);



        ubo.mMat = scale(mat4(1), vec3(3));
        ubo.mvpMat = ViewPrj * ubo.mMat;
        ubo.nMat = inverse(transpose(ubo.mMat));


        GlobalUniformBufferObject gubo{};
        static float L_time = 0.0f;
        L_time += deltaT;
        gubo.time = L_time;

        gubo.selector = vec3(showNormal ? 0 : 1, showNormal ? 1 : 0, 0);
        // rotate lightdir in time around z
        gubo.lightDir = normalize(vec3(0.0f, 0.0f, 0.0f));
        gubo.lightColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
        gubo.eyePos = cameraPos;


        DS1.map(currentImage, &ubo, sizeof(ubo), 0);
        DS1.map(currentImage, &gubo, sizeof(gubo), 1);
    }

    float size = 0.025f;

    float perlinNoise(float i, float j) {
        const siv::PerlinNoise::seed_type seed = 123456u;
        const siv::PerlinNoise perlin{seed};


        // Calculate the distance from the center of the grid (assuming it is centered at (0, 0))
        float x =(i-250);
        float y =(j-250);
        float distanceFromCenter = std::sqrt(x*x + y*y);

        // Define parameters for the Gaussian RBF
        float amplitude = 2.0f; // Amplitude of the RBF
        float sigmaSquared = 100.0f; // Variance of the RBF

        // Calculate a value using Perlin noise and Gaussian RBF with sigmoid smoothing
        float perlinValue = (float)perlin.octave2D_01(i*0.01f, j*0.01f, 4);
        float normalizedDistanceFromCenter = distanceFromCenter / 250.0f; // Normalize distance to range [0,1]
        normalizedDistanceFromCenter *= normalizedDistanceFromCenter; // Square to increase effect towards center

        return amplitude * perlinValue * std::exp(-normalizedDistanceFromCenter*distanceFromCenter / sigmaSquared);

        //return 2.0f * (float) perlin.octave2D_01((i * 0.01), (j * 0.01), 4);

        //return 2.0f * sin(i*0.01f);
    }

    void gamePhysics(float deltaT, vec3 m) {
        const float MOVE_SPEED = 1.0f;
        const float JUMP_SPEED = 15.0f; // Adjust this value to control jump height
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
            std::cout << "Jumping\n";
            // Only allow jumping if the object is very close to the ground (avoid double jumps)
            jumpTime = 5.0f;
        }

        if (jumpTime >= 0.0f) {
            velocity.y += jumpTime*jumpTime;
            jumpTime -= 0.5f;
        }

        // Update the position again after the jump
        Pos = Pos + velocity * deltaT;
    }

    void gameLogic(float deltaT, vec3 r) {
        // Parameters
        // Camera FOV-y, Near Plane and Far Plane
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

        // Integration with the timers and the controllers

        // Game Logic implementation
        // Current Player Position - statc variable make sure its value remain unchanged in subsequent calls to the procedure

        // To be done in the assignment
        ViewPrj = mat4(1);
        mat4 World = mat4(1);

        // World

        // Rotation
        Yaw = Yaw - ROT_SPEED * deltaT * r.y;
        Pitch = Pitch + ROT_SPEED * deltaT * r.x;
        Pitch = Pitch < minPitch ? minPitch :
                (Pitch > maxPitch ? maxPitch : Pitch);
        Roll = Roll - ROT_SPEED * deltaT * r.z;
        Roll = Roll < radians(-175.0f) ? radians(-175.0f) :
               (Roll > radians(175.0f) ? radians(175.0f) : Roll);

//std::cout << Pos.x << ", " << Pos.y << ", " << Pos.z << ", " << Yaw << ", " << Pitch << ", " << Roll << "\n";

        // Final world matrix computaiton
        World = translate(mat4(1), Pos) * rotate(mat4(1.0f), Yaw, vec3(0, 1, 0));

        // Projection
        mat4 Prj = perspective(FOVy, Ar, nearPlane, farPlane);
        Prj[1][1] *= -1;

        // View
        // Target
        vec3 target = Pos + vec3(0.0f, camHeight, 0.0f);

        // Camera position, depending on Yaw parameter, but not character direction
        cameraPos = World * vec4(0.0f, camHeight + camDist * sin(Pitch), camDist * cos(Pitch), 1.0);
        // Damping of camera
        mat4 View = rotate(mat4(1.0f), -Roll, vec3(0, 0, 1)) *
                    lookAt(cameraPos, target, vec3(0, 1, 0));

        ViewPrj = Prj * View;
    }

    void createGrid(std::vector<Vertex> &vDef, std::vector<uint32_t> &vIdx);

    void createFunctionMesh(std::vector<Vertex> &vDef, std::vector<uint32_t> &vIdx);

    void createCylinderMesh(std::vector<Vertex> &vDef, std::vector<uint32_t> &vIdx);
};

#include "primGen.hpp"


// This is the main: probably you do not need to touch this!
int main() {
    Main app;

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}