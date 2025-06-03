#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform int screenW;
uniform int screenH;

uniform vec3 cameraPos;
uniform vec3 cameraU; // right
uniform vec3 cameraV; // up
uniform vec3 cameraW; // fwd
uniform float focalLength;
uniform float viewportWidth;
uniform float viewportHeight;

struct Ray{
    vec3 p; // origin
    vec3 dir;
};

vec3 rayAt(Ray r, float t){
    return r.p + r.dir * t;
}

bool hit_sphere(vec3 center, float radius, Ray r){
    vec3 oc = center - r.p;
    float a = dot(r.dir, r.dir);
    float b = -2.0 * dot(r.dir, oc);
    float c = dot(oc, oc) - radius*radius;

    float disc = b*b - 4*a*c;
    return disc >= 0;
}

vec3 ray_color(Ray r){
    if(hit_sphere(vec3(0,0,-1),0.5,r)){
        return vec3(1.0,0.0,0.0);
    }

    return vec3(1-fragTexCoord.y, 1-fragTexCoord.y,1.0);
}

vec3 ray_dir(vec2 fragCoord, vec2 resolution) {
    vec2 uv = (fragCoord / resolution) * 2.0 - 1.0;
    vec3 dir = normalize(-focalLength * cameraW + uv.x * viewportWidth * cameraU + uv.y * viewportHeight * cameraV);
    return dir;
}

void main(){
    vec2 res = vec2(screenW,screenH);
    vec2 fragCoord = gl_FragCoord.xy;

    Ray currRay = Ray(cameraPos, ray_dir(fragCoord, res));

    finalColor = vec4(ray_color(currRay), 1.0);
}
