#version 330

const int MAX_SPHERES = 128;

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

uniform vec3 sphereCenters[MAX_SPHERES];
uniform float sphereRadii[MAX_SPHERES];
uniform int sphereCount;

struct Ray{
    vec3 p; // origin
    vec3 dir;
};

vec3 rayAt(Ray r, float t){
    return r.p + r.dir * t;
}

float hit_sphere(vec3 center, float radius, Ray r){
    vec3 oc = center - r.p;
    float a = dot(r.dir, r.dir);
    float h = dot(r.dir, oc);
    float c = dot(oc, oc) - radius*radius;

    float disc = h*h - a*c;

    if(disc < 0){
        return -1.0;
    }
    return (h - sqrt(disc))/a;
}

vec3 ray_color(Ray r){
    for(int i = 0; i < sphereCount; i++){
        float t = hit_sphere(sphereCenters[i], sphereRadii[i], r);
        if(t > 0){
            vec3 norm = normalize(rayAt(r, t) - sphereCenters[i]);
            return 0.5*(vec3(1,1,1)+norm);
        }
    }
    float a = 0.5*(fragTexCoord.y+1);
    return (1-a)*vec3(1,1,1)+a*vec3(0.5,0.7,1.0);
}

vec3 ray_dir(vec2 fragCoord) {
    vec3 topright = vec3(viewportWidth*cameraU);
    vec3 botleft = vec3(viewportHeight*cameraV);

    vec3 pixeldu = vec3(topright/screenW);
    vec3 pixeldv = vec3(botleft/screenH);

    vec3 topleft = cameraPos - focalLength*cameraW - topright/2 - botleft/2;

    vec3 pixel00 = topleft + 0.5*(pixeldu+pixeldv);

    vec3 pixelCenter = pixel00 + fragCoord.x * pixeldu + fragCoord.y * pixeldv;

    vec3 dir = normalize(pixelCenter - cameraPos);
    return dir;
}

void main(){
    vec2 fragCoord = gl_FragCoord.xy;

    Ray currRay = Ray(cameraPos, ray_dir(fragCoord));

    finalColor = vec4(ray_color(currRay), 1.0);
}
