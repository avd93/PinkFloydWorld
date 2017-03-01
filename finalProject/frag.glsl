#version 330

uniform sampler2D tex;

in vec3 fcolor;
in vec3 fnormal;
in vec3 fposition;
in vec2 fuv;

uniform vec3 lightPos;
uniform vec3 lightColor;

uniform float ambient;
uniform float diffuse;
uniform float specIntensity;

uniform mat4 view;

out vec4 color_out;

void main() {
    vec4 tempPosition = inverse(view)*vec4(lightPos,1);
    vec3 L = normalize(tempPosition.xyz - fposition);

    // calculate diffuse and ambient values
    float diffuseColor = diffuse*dot(L,fnormal);
    vec3 diff = diffuseColor*lightColor;
    vec3 ambientStrength = ambient*lightColor;

    // caclulate specular
    vec3 cam = vec3(0,0,view[3][2]);
    vec3 viewDirection = normalize(cam-fposition);
    vec3 reflectDirection = reflect(-L,fnormal);
    float spec = pow(max(dot(viewDirection,reflectDirection),0.),specIntensity);
    vec3 specular = (0.5f)*spec*lightColor;

//  color_out = texture(tex,fuv).rrra;
//  color_out = vec4(fuv,0,1);
//    color_out = vec4(fcolor,1);
    color_out = vec4((ambientStrength + diff +  specular)*fcolor, 1);
}
