#version 330

//uniform mat4 u_MVPMatrix;      // A constant representing the combined model/view/projection matrix.
//uniform mat4 u_MVMatrix;       // A constant representing the combined model/view matrix.

uniform mat4 u_PMatrix;
uniform mat4 u_VMatrix;
uniform mat4 u_MMatrix;

//attribute vec4 a_Position;     
//attribute vec4 a_Color;        
//attribute vec3 a_Normal;       
layout(location = 0) in vec3 a_Position;	// Per-vertex position information we will pass in.
layout(location = 1) in vec4 a_Color;		// Per-vertex color information we will pass in.
layout(location = 2) in vec3 a_Normal;		// Per-vertex normal information we will pass in.
layout(location = 3) in vec2 a_TexCoord;
layout(location = 4) in vec2 a_LightMag;
 
out vec3 v_Position;       // This will be passed into the fragment shader.
out vec4 v_Color;          // This will be passed into the fragment shader.
out vec3 v_Normal;         // This will be passed into the fragment shader.
out vec2 v_TexCoord;
 
out vec3 v_LightPos0;

// The entry point for our vertex shader.
// pvm p*v*m
void main()
{

	mat4 u_MVPMatrix = u_PMatrix * u_VMatrix * u_MMatrix;
	mat4 u_MVMatrix = u_VMatrix * u_MMatrix;

    // Transform the vertex into eye space.
    v_Position = vec3(u_MVMatrix * vec4(a_Position,1.0)).xyz;
	v_LightPos0 = vec3(u_MVMatrix * vec4(-10.0f, 8.0f, -20.0f, 1.0f)).xyz;
 
    // Pass through the color.
    v_Color = a_Color;
 
	// Pass the tex coord
	v_TexCoord = a_TexCoord;

    // Transform the normal's orientation into eye space.
    v_Normal = vec3(u_MVMatrix * vec4(a_Normal, 0.0));
 
    // gl_Position is a special variable used to store the final position.
    // Multiply the vertex by the matrix to get the final point in normalized screen coordinates.
    gl_Position = u_MVPMatrix * vec4(a_Position,1.0);
}
