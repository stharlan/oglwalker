#version 330

//precision mediump float;       // Set the default precision to medium. We don't need as high of a
                               // precision in the fragment shader.
uniform vec3 u_LightPos;       // The position of the light in eye space.
 
in vec3 v_Position;       // Interpolated position for this fragment.
in vec4 v_Color;          // This is the color from the vertex shader interpolated across the
                               // triangle per fragment.
in vec3 v_Normal;         // Interpolated normal for this fragment.
in vec2 v_TexCoord;
in vec3 v_LightPos0;
in vec3 v_LightPos1;
in vec3 v_LightPos2;
in vec3 v_LightPos3;

uniform sampler2D gSampler;

// The entry point for our fragment shader.
void main()
{
    // Will be used for attenuation.
	// v_Position is the pixel position
    float distance = length(u_LightPos - v_Position);
	float distance_light0 = length(v_LightPos0 - v_Position);
	float distance_light1 = length(v_LightPos1 - v_Position);
	float distance_light2 = length(v_LightPos2 - v_Position);
	float distance_light3 = length(v_LightPos3 - v_Position);
 
    // Get a lighting direction vector from the light to the vertex.
    vec3 lightVector = normalize(u_LightPos - v_Position);
	vec3 lightVector0 = normalize(v_LightPos0 - v_Position);
	vec3 lightVector1 = normalize(v_LightPos1 - v_Position);
	vec3 lightVector2 = normalize(v_LightPos2 - v_Position);
	vec3 lightVector3 = normalize(v_LightPos3 - v_Position);
 
    // Calculate the dot product of the light vector and vertex normal. If the normal and light vector are
    // pointing in the same direction then it will get max illumination.
    float diffuse = max(dot(v_Normal, lightVector), 0.1);
	float diffuse0 = max(dot(v_Normal, lightVector0), 0.1);
	float diffuse1 = max(dot(v_Normal, lightVector1), 0.1);
	float diffuse2 = max(dot(v_Normal, lightVector2), 0.1);
	float diffuse3 = max(dot(v_Normal, lightVector3), 0.1);
 
    // Add attenuation.
    diffuse = diffuse * (1.0 / (1.0 + (0.01 * distance * distance)));
	diffuse0 = diffuse0 * (1.0 / (1.0 + (0.001 * distance_light0 * distance_light0)));
	diffuse1 = diffuse1 * (1.0 / (1.0 + (0.001 * distance_light1 * distance_light1)));
	diffuse2 = diffuse2 * (1.0 / (1.0 + (0.001 * distance_light2 * distance_light2)));
	diffuse3 = diffuse3 * (1.0 / (1.0 + (0.001 * distance_light3 * distance_light3)));
	diffuse = max(diffuse, diffuse0);
	diffuse = max(diffuse, diffuse1);
	diffuse = max(diffuse, diffuse2);
	diffuse = max(diffuse, diffuse3);
 
    // Multiply the color by the diffuse illumination level to get final output color.
    //gl_FragColor = v_Color * diffuse;
	gl_FragColor = texture2D(gSampler, v_TexCoord) * diffuse;
}
