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

uniform sampler2D gSampler;

// The entry point for our fragment shader.
void main()
{
    // Will be used for attenuation.
	// v_Position is the pixel position
    float distance = length(u_LightPos - v_Position);
	float distance_light0 = length(v_LightPos0 - v_Position);
 
    // Get a lighting direction vector from the light to the vertex.
    vec3 lightVector = normalize(u_LightPos - v_Position);
	vec3 lightVector0 = normalize(v_LightPos0 - v_Position);
 
    // Calculate the dot product of the light vector and vertex normal. If the normal and light vector are
    // pointing in the same direction then it will get max illumination.
    float diffuse = max(dot(v_Normal, lightVector), 0.1);
	float diffuse0 = max(dot(v_Normal, lightVector0), 0.1);
 
    // Add attenuation.
    diffuse = diffuse * (1.0 / (1.0 + (0.01 * distance * distance)));
	diffuse0 = diffuse0 * (1.0 / (1.0 + (0.001 * distance_light0 * distance_light0)));
	diffuse = max(diffuse, diffuse0);
 
    // Multiply the color by the diffuse illumination level to get final output color.
    //gl_FragColor = v_Color * diffuse;
	gl_FragColor = texture2D(gSampler, v_TexCoord) * diffuse;
}
