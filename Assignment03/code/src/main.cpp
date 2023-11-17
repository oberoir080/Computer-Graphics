//Assignment 03: Lighting and shading

/*References
  Trackball: http://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Arcball
*/
using namespace::std;
#include "utils.h"

#define  GLM_FORCE_RADIANS
#define  GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

//Globals
int screen_width = 640, screen_height=640;
GLint vModel_uniform, vView_uniform, vProjection_uniform;

// GLint vColor_uniform;
glm::mat4 modelT, viewT, projectionT;//The model, view and projection transformations
GLint lpos_world_uniform, eye_normal_uniform;

double oldX, oldY, currentX, currentY;
bool isDragging=false;

void createCubeObject(unsigned int &, unsigned int &);

void setupModelTransformation(unsigned int &);
void setupViewTransformation(unsigned int &);
void setupProjectionTransformation(unsigned int &);
glm::vec3 getTrackBallVector(double x, double y);

int main(int, char**)
{
    // Setup window
    GLFWwindow *window = setupWindow(screen_width, screen_height);
    ImGuiIO& io = ImGui::GetIO(); // Create IO object

    ImVec4 clearColor = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);

    unsigned int shaderProgram = createProgram("./shaders/vshader.vs", "./shaders/fshaderB.fs"); 
    
    //Change vshader to vshaderNotSpotlight for part 1 
    //Change fshader to fShaderB for part 2B


    //Get handle to color variable in shader
    // vColor_uniform = glGetUniformLocation(shaderProgram, "vColor");
    // if(vColor_uniform == -1){
    //     fprintf(stderr, "Could not bind location: vColor\n");
    //     exit(0);
    // }

    lpos_world_uniform = glGetUniformLocation(shaderProgram, "lpos_world");
    if(lpos_world_uniform == -1){
        fprintf(stderr, "Could not bind location: lpos_world\n");
    }

    //Get handle to eye normal variable in shader
    eye_normal_uniform = glGetUniformLocation(shaderProgram, "eye_normal");
    if(eye_normal_uniform == -1){
        fprintf(stderr, "Could not bind location: eye_normal. Specular Lighting Switched Off.\n");
    }

    glUseProgram(shaderProgram);

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);

    setupModelTransformation(shaderProgram);
    setupViewTransformation(shaderProgram);
    setupProjectionTransformation(shaderProgram);

    createCubeObject(shaderProgram, VAO);

    oldX = oldY = currentX = currentY = 0.0;
    int prevLeftButtonState = GLFW_RELEASE;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Get current mouse position
        int leftButtonState = glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT);
        double x,y;
        glfwGetCursorPos(window,&x,&y);
        if(leftButtonState == GLFW_PRESS && prevLeftButtonState == GLFW_RELEASE){
            isDragging = true;
            currentX = oldX = x;
            currentY = oldY = y;
        }
        else if(leftButtonState == GLFW_PRESS && prevLeftButtonState == GLFW_PRESS){
            currentX = x;
            currentY = y;
        }
        else if(leftButtonState == GLFW_RELEASE && prevLeftButtonState == GLFW_PRESS){
            isDragging = false;
        }

        // Rotate based on mouse drag movement
        prevLeftButtonState = leftButtonState;
        if(isDragging && (currentX !=oldX || currentY != oldY))
        {
            glm::vec3 va = getTrackBallVector(oldX, oldY);
            glm::vec3 vb = getTrackBallVector(currentX, currentY);

            float angle = acos(std::min(1.0f, glm::dot(va,vb)));
            glm::vec3 axis_in_camera_coord = glm::cross(va, vb);
            glm::mat3 camera2object = glm::inverse(glm::mat3(viewT*modelT));
            glm::vec3 axis_in_object_coord = camera2object * axis_in_camera_coord;
            modelT = glm::rotate(modelT, angle, axis_in_object_coord);
            glUniformMatrix4fv(vModel_uniform, 1, GL_FALSE, glm::value_ptr(modelT));

            oldX = currentX;
            oldY = currentY;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glUseProgram(shaderProgram);

        {
            ImGui::Begin("Information");                          
            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(VAO); 

        glUniform3f(lpos_world_uniform, 200.0, 200.0, 200.0);
        glUniform3f(eye_normal_uniform, 200.0, -200.0, 200.0);  //-40.0, 40.0);
        
        // glUniform3f(vColor_uniform, 0.5, 0.5, 0.5);
        glDrawArrays(GL_TRIANGLES, 0, 15000);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

    }

    // Cleanup
    cleanup(window);

    return 0;
}

void createCubeObject(unsigned int &program, unsigned int &cube_VAO)
{
    glUseProgram(program);

    //Bind shader variables
    int vVertex_attrib = glGetAttribLocation(program, "vVertex");
    if(vVertex_attrib == -1) {
        fprintf(stderr, "Could not bind location: vVertex\n");
        exit(0);
    }

    int vNormal_attrib = glGetAttribLocation(program, "vNormal");
    if (vNormal_attrib == -1)
    {
        std::cout << "Could not bind location: vNormal\n";
    }

    
    // Define the parameters of the sphere
    const int slices = 50;
    const int stacks = 50;
    const float radius = 20.0f;

    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;

    std::vector<GLfloat> Normalvertex;
    std::vector<GLuint> Normalindex;

    // Generate sphere vertices and indices
    for (int i = 0; i <= stacks; ++i) {
        float phi = (static_cast<float>(i) / stacks) * glm::pi<float>(); //u
        for (int j = 0; j <= slices; ++j) {
            float theta = (static_cast<float>(j) / slices) * 2.0f * glm::pi<float>(); //v

            float x = radius * sin(theta) * sin(phi);  //i
            float y = radius * cos(phi); //j
            float z = radius * cos(theta) * sin(phi);  //k

             // i + j + k = f
            // diff with phi
            // diff with theta

            // rsinthsinphi + rcosphi + rcosthetasinphi
            // rcosphisinth - rsinphi + rcosthetacosphi
            // rcosthetasinphi + 0 -rsinthetasinphi
            
            glm::vec3 delf_delu={radius*cos(phi)*sin(theta),-radius*sin(phi),radius*cos(phi)*cos(theta)};
            glm::vec3 delf_delv={radius*cos(theta)*sin(phi),0,-radius*sin(theta)*sin(phi)};

            glm::vec3 cross_product = glm::cross(delf_delu, delf_delv);

            cross_product = glm::normalize(cross_product);
           

            float dot_x = cross_product.x; 
            float dot_y = cross_product.y;
            float dot_z = cross_product.z;

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            Normalvertex.push_back(dot_x);
            Normalvertex.push_back(dot_y);
            Normalvertex.push_back(dot_z);
        }
    }

    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int curr = i * (slices + 1) + j;
            int next = curr + 1;
            int below = curr + slices + 1;
            int belowNext = below + 1;

            indices.push_back(curr);
            indices.push_back(below);
            indices.push_back(next);

            indices.push_back(next);
            indices.push_back(below);
            indices.push_back(belowNext);

            Normalindex.push_back(curr);
            Normalindex.push_back(below);
            Normalindex.push_back(next);

            Normalindex.push_back(next);
            Normalindex.push_back(below);
            Normalindex.push_back(belowNext);
        }
    }

    GLfloat *cube_vertices = vertices.data();
    GLuint *cube_indices = indices.data();

    GLfloat *Normalvertices = Normalvertex.data();
    GLuint *Normalindices = Normalindex.data();

    // size_t indices_size = sizeof(indices);
    // size_t vertices_size = sizeof(vertices);
    // cout << indices_size;
    // cout << endl;
    // cout << vertices_size;
    // cout << endl;

    glGenVertexArrays(1, &cube_VAO);
    glBindVertexArray(cube_VAO);
    
    int nVertices = 15000; 

    
    GLfloat *expanded_vertices = new GLfloat[nVertices*3];
    GLfloat *expanded_normals = new GLfloat[nVertices*3];
    for(int i=0; i<nVertices; ++i) {
        expanded_vertices[i*3] = cube_vertices[cube_indices[i]*3];
        expanded_vertices[i*3 + 1] = cube_vertices[cube_indices[i]*3+1];
        expanded_vertices[i*3 + 2] = cube_vertices[cube_indices[i]*3+2];

        expanded_normals[i*3] = Normalvertices[Normalindices[i]*3];
        expanded_normals[i*3 + 1] = Normalvertices[Normalindices[i]*3+1];
        expanded_normals[i*3 + 2] = Normalvertices[Normalindices[i]*3+2];
    }
    GLuint vertex_VBO;
    GLuint normal_VBO;
    

    glGenBuffers(1, &vertex_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_VBO);
    glBufferData(GL_ARRAY_BUFFER, nVertices*3*sizeof(GLfloat), expanded_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vVertex_attrib);
    glVertexAttribPointer(vVertex_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    delete []expanded_vertices;

    glGenBuffers(1, &normal_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, normal_VBO);
    glBufferData(GL_ARRAY_BUFFER, nVertices*3*sizeof(GLfloat), expanded_normals, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vNormal_attrib);
    glVertexAttribPointer(vNormal_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    delete []expanded_normals;

    

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); //Unbind the VAO to disable changes outside this function.
}

void setupModelTransformation(unsigned int &program)
{
    //Modelling transformations (Model -> World coordinates)
    modelT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0));//Model coordinates are the world coordinates

    //Pass on the modelling matrix to the vertex shader
    glUseProgram(program);
    vModel_uniform = glGetUniformLocation(program, "vModel");
    if(vModel_uniform == -1){
        fprintf(stderr, "Could not bind location: vModel\n");
        exit(0);
    }
    glUniformMatrix4fv(vModel_uniform, 1, GL_FALSE, glm::value_ptr(modelT));
}
















void setupViewTransformation(unsigned int &program)
{
    //Viewing transformations (World -> Camera coordinates
    //Camera at (0, 0, 100) looking down the negative Z-axis in a right handed coordinate system
    viewT = glm::lookAt(glm::vec3(40.0, -40.0, 40.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

    //Pass-on the viewing matrix to the vertex shader
    glUseProgram(program);
    vView_uniform = glGetUniformLocation(program, "vView");
    if(vView_uniform == -1){
        fprintf(stderr, "Could not bind location: vView\n");
        exit(0);
    }
    glUniformMatrix4fv(vView_uniform, 1, GL_FALSE, glm::value_ptr(viewT));
}

void setupProjectionTransformation(unsigned int &program)
{
    //Projection transformation
    projectionT = glm::perspective(45.0f, (GLfloat)screen_width/(GLfloat)screen_height, 0.1f, 1000.0f);

    //Pass on the projection matrix to the vertex shader
    glUseProgram(program);
    vProjection_uniform = glGetUniformLocation(program, "vProjection");
    if(vProjection_uniform == -1){
        fprintf(stderr, "Could not bind location: vProjection\n");
        exit(0);
    }
    glUniformMatrix4fv(vProjection_uniform, 1, GL_FALSE, glm::value_ptr(projectionT));
}

glm::vec3 getTrackBallVector(double x, double y)
{
	glm::vec3 p = glm::vec3(2.0*x/screen_width - 1.0, 2.0*y/screen_height - 1.0, 0.0); //Normalize to [-1, +1]
	p.y = -p.y; //Invert Y since screen coordinate and OpenGL coordinates have different Y directions.

	float mag2 = p.x*p.x + p.y*p.y;
	if(mag2 <= 1.0f)
		p.z = sqrtf(1.0f - mag2);
	else
		p = glm::normalize(p); //Nearest point, close to the sides of the trackball
	return p;
}

