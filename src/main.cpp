#include <iostream>
#include <flgl.h>
#include <vector>
#include "../lib/flgl/src/Verticies.h"
#include "../lib/flgl/src/tools/Camera.h"
#include "../lib/flgl/src/tools/flgl_math.h"
using std::vector;
using namespace glm;
static Window& window = flgl::window;

static vector<Vt_classic> qverts = { 
	{{-1.,-1., 0.}, {0.,0.}},
	{{-1., 1., 0.}, {0.,1.}},
	{{ 1., 1., 0.}, {1.,1.}},
	{{ 1.,-1., 0.}, {1.,0.}} 
};

static vector<uint32_t> qelems = {0, 1, 2,   0, 3, 2};
static VertexArray quad;
void setup_quad_vao() {
	quad.create();
	VertexBuffer<Vt_classic> vbo;
	ElementBuffer ibo;
	quad.bind();
	vbo.create();
	vbo.bind();
	quad.attach(vbo);
	vbo.buffer(qverts);

	ibo.create();
	ibo.bind();
	ibo.buffer_data(qelems);
	quad.unbind();
	vbo.destroy();
	ibo.destroy();
}

void createCylinder(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<uint32_t>& indices, int numCircleVertices, float height) {
    float radius = 1.0f; // You can modify this as needed
    float halfHeight = height / 2.0f;

    // Clear vectors
    vertices.clear();
    normals.clear();
    indices.clear();

    // Generate vertices for side surface
    for (int i = 0; i < 2; ++i) {
        float y = i * height - halfHeight;
        for (int j = 0; j < numCircleVertices; ++j) {
            float angle = 2.0f * M_PI * j / numCircleVertices;
            float x = radius * cos(angle);
            float z = radius * sin(angle);
            vertices.push_back(glm::vec3(x, y, z));

            // Compute normal for side (without considering y-component)
            glm::vec3 normal = glm::normalize(glm::vec3(x, 0.0f, z));
            normals.push_back(normal);
        }
    }

    // Add center vertices for the top and bottom faces
    vertices.push_back(glm::vec3(0.0f, halfHeight, 0.0f)); // Top center
    normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));       // Normal pointing upwards

    vertices.push_back(glm::vec3(0.0f, -halfHeight, 0.0f)); // Bottom center
    normals.push_back(glm::vec3(0.0f, -1.0f, 0.0f));        // Normal pointing downwards

    // Generate indices for the side surface
    for (int i = 0; i < numCircleVertices; ++i) {
        int next = (i + 1) % numCircleVertices;

        // Two triangles per quad
        indices.push_back(i);
        indices.push_back(i + numCircleVertices);
        indices.push_back(next + numCircleVertices);

        indices.push_back(i);
        indices.push_back(next + numCircleVertices);
        indices.push_back(next);
    }

    // Generate indices for the top and bottom faces
    int topCenterIndex = 2 * numCircleVertices;
    int bottomCenterIndex = topCenterIndex + 1;

    for (int i = 0; i < numCircleVertices; ++i) {
        int next = (i + 1) % numCircleVertices;

        // Top face
        indices.push_back(bottomCenterIndex);
        indices.push_back(i);
        indices.push_back(next);

        // Bottom face
        indices.push_back(topCenterIndex);
        indices.push_back(i + numCircleVertices);
        indices.push_back(next + numCircleVertices);
    }
}


struct rbfVt {
	vec3 pos, normal;
};

template <>
void VertexBuffer<rbfVt>::attach_to_vao(VertexArray const& vao) const {
	vao.bind();
	this->bind();
	vao.attrib(0,							// layout index
			   3, GL_FLOAT,					// dimension and repr
			   sizeof(rbfVt), 				// size of vertex
			   offsetof(rbfVt,pos));		// offset of member

	vao.attrib(1,							// layout index
			   3, GL_FLOAT,					// dimension and repr
			   sizeof(rbfVt), 				// size of vertex
			   offsetof(rbfVt,normal));		// offset of member
}

static VertexArray cyl;
static void setup_cyl() {
	vector<vec3> vvec;
	vector<vec3> nvec;
	vector<uint32_t> ivec;
	createCylinder(vvec, nvec, ivec, 16, 2);
	vector<rbfVt> rbfVec;
	std::cout << "indices vector size: " << ivec.size() << std::endl;
	for (uint32_t i = 0; i < vvec.size(); i++) {
		rbfVec.push_back({{vvec[i]}, {nvec[i]}});
	}
	cyl.create();
	cyl.bind();
	VertexBuffer<rbfVt> vbo;
	vbo.create();
	cyl.attach(vbo);
	vbo.buffer(rbfVec);

	ElementBuffer ibo;
	ibo.create(); ibo.bind();
	ibo.buffer_data(ivec);
	cyl.unbind();
	vbo.destroy();
	ibo.destroy();
}


static PerspectiveCamera cam(vec3(0, 0, -1), vec3(0, 0, 1), vec3(0, 1, 0), 0.001, 10000, 60);
static glm::mat4 qmmat = genModelMat3d(vec3(0, -1, 10), vec3(90, 0, 0), vec3(16*3, 20, 1));
static Texture grass;

static mat4 cylmat = genModelMat3d(vec3(0., 0., 10.), vec3(0.), vec3(0.5, 2, 0.5));

static mat4 cylmat_first_base = genModelMat3d(vec3(-2., -1., 6.), vec3(0.), vec3(0.25, 0.05, 0.25));
static mat4 cylmat_second_base = genModelMat3d(vec3(0., -1., 8.), vec3(0.), vec3(0.25, 0.05, 0.25));
static mat4 cylmat_third_base = genModelMat3d(vec3(2, -1., 6.), vec3(0.), vec3(0.25, 0.05, 0.25));
static mat4 cylmat_home_base = genModelMat3d(vec3(0., -1., 2), vec3(0.), vec3(0.25, 0.05, 0.25));


static mat4 pitcher = genModelMat3d(vec3(0., -0.5, 6), vec3(0.), vec3(0.05, 0.5, 0.05));

static mat4 ball = genModelMat3d(vec3(0.08, -0.5, 6), vec3(0.), vec3(0.05, 0.05, 0.05));

int main() {

	flgl::gl.init();
	flgl::config.set_shader_path("shad/");
	flgl::config.set_asset_path("assets/");
	flgl::window.create("rbf baseball", 1280, 1280);
	flgl::gl.set_clear_color(0.3, 0.6, 0.9, 1);
	flgl::gl.enable_depth_test(true);
	grass = Texture::from_file("grass", true);

	setup_quad_vao();
	setup_cyl();
	Shader grass_shader = Shader::from_source("vert", "grass_frag");
	Shader cyl_shader = Shader::from_source("cyl_vert", "cylinder");
	cam.update();
	bool wf = false;
	vec3 cameraPos = vec3(0., 0., 0.);
	float cameraVelocity = 0.1;

	float ballX = 0.08, ballY = -0.5, ballZ = 6;

	// cam.setMouseControl(true);
	// window.grab_mouse();

	float ballVelocity = 0.01;

	while (!window.should_close()) { 
		flgl::gl.clear();
		if (window.keyboard[GLFW_KEY_ESCAPE].pressed) break;
		if (window.keyboard[GLFW_KEY_K].pressed) {
			flgl::gl.wireframe(wf); wf = !wf;
		}
		if (window.keyboard[GLFW_KEY_W].down) {
			cameraPos.z += cameraVelocity;
		}
		if (window.keyboard[GLFW_KEY_S].down) {
			cameraPos.z -= cameraVelocity;
		}
		if (window.keyboard[GLFW_KEY_A].down) {
			cameraPos.x += cameraVelocity;
		}
		if (window.keyboard[GLFW_KEY_D].down) {
			cameraPos.x -= cameraVelocity;
		}
		if (window.keyboard[GLFW_KEY_Q].down) {
			cameraPos.y += cameraVelocity;
		}
		if (window.keyboard[GLFW_KEY_E].down) {
			cameraPos.y -= cameraVelocity;
		}

		if (window.keyboard[GLFW_KEY_SPACE].pressed) {
			static int score = 0;
			if (ballZ >= 1.5 && ballZ <= 2.5) {
				std::cout << "HIT! score: " << ++score << std::endl;
				ballZ = 6;
				ballVelocity *= 2;
			} else {
				std::cout << "STRIKE" << std::endl;
				*((char*)0) = 1;
			}
		}
		ballZ -= ballVelocity;
		
		cam.getPos() = cameraPos;
		cam.update();

		ball = genModelMat3d(vec3(ballX, ballY, ballZ), vec3(0.), vec3(0.05, 0.05, 0.05));


		quad.bind();
		grass_shader.bind();
		grass_shader.uMat4("uProj", cam.proj());
		grass_shader.uMat4("uView", cam.view());
		grass_shader.uMat4("uModel", qmmat);

		grass.bind();

		flgl::gl.draw_elements(GL_TRIANGLES, 6, GL_UNSIGNED_INT);

		cyl_shader.bind();
		cyl_shader.uVec3("uLightDirection", glm::vec3(0.0, 1.0, 0.0)); // Change this direction as needed
		cyl_shader.uMat4("uProj", cam.proj());
		cyl_shader.uMat4("uView", cam.view());
		cyl_shader.uMat4("uModel", cylmat);


		// cyl.bind();
		// flgl::gl.draw_elements(GL_TRIANGLES, 16*6, GL_UNSIGNED_INT);

		cyl_shader.uMat4("uModel", cylmat_first_base);
		cyl.bind();
		flgl::gl.draw_elements(GL_TRIANGLES, 16*6+96, GL_UNSIGNED_INT);
		cyl_shader.uMat4("uModel", cylmat_second_base);
		cyl.bind();
		flgl::gl.draw_elements(GL_TRIANGLES, 16*6+96, GL_UNSIGNED_INT);
		cyl_shader.uMat4("uModel", cylmat_third_base);
		cyl.bind();
		flgl::gl.draw_elements(GL_TRIANGLES, 16*6+96, GL_UNSIGNED_INT);
		cyl_shader.uMat4("uModel", cylmat_home_base);
		cyl.bind();
		flgl::gl.draw_elements(GL_TRIANGLES, 16*6+96, GL_UNSIGNED_INT);
		cyl_shader.uMat4("uModel", ball);
		cyl.bind();
		flgl::gl.draw_elements(GL_TRIANGLES, 16*6+96, GL_UNSIGNED_INT);
		cyl_shader.uMat4("uModel", pitcher);
		cyl.bind();
		flgl::gl.draw_elements(GL_TRIANGLES, 16*6+96, GL_UNSIGNED_INT);

		window.update();
	}

	grass_shader.destroy();
	cyl_shader.destroy();
	cyl.destroy();
	quad.destroy();
	flgl::window.destroy();
	flgl::gl.destroy();


	return 0;
}


