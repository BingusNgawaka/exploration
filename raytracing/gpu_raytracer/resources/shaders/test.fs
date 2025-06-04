#version 330

const int MAX_SPHERES = 128;
const float INFINITY = 3.402823466e+38;

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform int screenW;
uniform int screenH;
uniform int samplesPerPixel;
uniform int maxDepth;
uniform float time;

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

#define LAMBERTIAN 0
#define METAL 1
#define DIELECTRIC 2

uniform int materialTypes[MAX_SPHERES]; 
uniform vec3 materialCols[MAX_SPHERES];
uniform float materialFuzz[MAX_SPHERES];
uniform float materialRefractionIndices[MAX_SPHERES];
uniform float materialEmissionStrs[MAX_SPHERES];

struct Material{
    int type;
    vec3 col;
    float fuzz;
    float refractionIndex;
    float emissionStr;
};

Material getMaterial(int index){
    return Material(materialTypes[index], materialCols[index], materialFuzz[index], materialRefractionIndices[index], materialEmissionStrs[index]);
}

struct Ray{
    vec3 p; // origin
    vec3 dir;
};

vec3 rayAt(Ray r, float t){
    return r.p + r.dir * t;
}

// RNG STUFF //
// slight modification of this:
// https://observablehq.com/@riccardoscalco/pcg-random-number-generators-in-glsl
uint pcg(uint v) {
    uint state = v * uint(747796405) + uint(2891336453);
    uint word = ((state >> ((state >> uint(28)) + uint(4))) ^ state) * uint(277803737);
    return (word >> uint(22)) ^ word;
}

float prng (inout uint p) {
    p = pcg(p);
    return float(pcg(p)) / float(4294967295.0f);
}

uint getSeed(vec2 coord){
    return uint(gl_FragCoord.x) * 1973u + uint(gl_FragCoord.y) * 9277u + 12345u;
}
uint seed = getSeed(gl_FragCoord.xy) ^ uint(time * 1000.0);


vec2 rand_square(float variance){
    return variance*vec2(prng(seed)-0.5, prng(seed)-0.5);
}

// ======== //

vec3 reflect(vec3 v, vec3 n){
    return v - 2*dot(v,n)*n;
}
vec3 refract(vec3 v, vec3 n, float refractionIndex){
    float cosTheta = min(dot(-v,n), 1);
    vec3 r_out_perp = refractionIndex*(v + cosTheta*n);
    vec3 r_out_parallel = -sqrt(abs(1-dot(r_out_perp,r_out_perp)))*n;
    return r_out_perp + r_out_parallel;
}
float reflectance(float cosine, float ri){
    // schlicks approx
    float r0 = (1-ri)/(1+ri);
    r0 = r0*r0;
    return r0 + (1-r0)*pow(1-cosine,5);
}

struct HitRec{
    vec3 p;
    vec3 norm;
    float t;
    Material mat;
    bool frontFace;
};

struct Sphere{
    vec3 center;
    float radius;
    Material mat;
};

bool hit_sphere(Sphere sphere, Ray r, float tmin, float tmax, inout HitRec rec){
    vec3 oc = sphere.center - r.p;
    float a = dot(r.dir, r.dir);
    float h = dot(r.dir, oc);
    float c = dot(oc, oc) - sphere.radius*sphere.radius;

    float disc = h*h - a*c;

    if(disc < tmin){
        return false;
    }

    float sqrtDisc = sqrt(disc);
    float root = (h-sqrtDisc)/a;
    if(root <= tmin || root >= tmax){
        root = (h+sqrtDisc)/a;
        if(root <= tmin || root >= tmax){
            return false;
        }
    }

    rec.t = root;
    rec.p = rayAt(r, rec.t);
    rec.mat = sphere.mat;

    vec3 outNorm = (rec.p - sphere.center)/sphere.radius;
    rec.frontFace = dot(r.dir, outNorm) < 0;

    if(rec.frontFace){
        rec.norm = outNorm;
    }else{
        rec.norm = -outNorm;
    }

    return true;
}

bool world_hit(Ray r, float tmin, float tmax, inout HitRec rec){
    HitRec tempRec = HitRec(vec3(0,0,0),vec3(0,0,0),0,Material(0,vec3(0),0,0,0),false);
    bool hitSomething = false;
    float closestT = tmax;

    // loop thru spheres and test ray collis, saving closest one
    for(int i = 0; i < sphereCount; i++){
        Sphere currSphere = Sphere(sphereCenters[i], sphereRadii[i], getMaterial(i));
        if(hit_sphere(currSphere, r, tmin, closestT, tempRec)){
            hitSomething = true;
            closestT = tempRec.t;
            rec = tempRec;
        }
    }
    return hitSomething;
}

vec3 rand_unit_vec(){
    while(true){
        float rx = prng(seed)*2 - 1;
        float ry = prng(seed)*2 - 1;
        float rz = prng(seed)*2 - 1;
        vec3 p = vec3(rx, ry, rz);
        float lensq = dot(p,p);

        if(lensq <= 1 && lensq > 1e-160){
            return p/sqrt(lensq);
        }
    }
}

vec3 ray_color(Ray r){
    vec3 col = vec3(1.0);
    vec3 incomingLight = vec3(0);

    for(int currDepth = 0; currDepth < maxDepth; currDepth++){
        HitRec rec = HitRec(vec3(0,0,0),vec3(0,0,0),0,Material(0,vec3(0),0,0,0), false);
        if(world_hit(r, 0.001, INFINITY, rec)){
            vec3 dir;
            vec3 udir = normalize(r.dir);
            if(rec.mat.type == LAMBERTIAN){
                dir = rec.norm + rand_unit_vec();
            }else if(rec.mat.type == METAL){
                dir = reflect(udir, rec.norm) + rec.mat.fuzz*rand_unit_vec();
            }else if(rec.mat.type == DIELECTRIC){
                float ri = rec.mat.refractionIndex;
                if(rec.frontFace){
                    ri = 1.0/rec.mat.refractionIndex;
                }

                float cos_theta = min(dot(-udir, rec.norm), 1.0);
                float sin_theta = sqrt(1-cos_theta*cos_theta);

                bool cannotRefract = ri * sin_theta > 1.0;
                if(cannotRefract || reflectance(cos_theta, ri) > prng(seed)){
                    dir = reflect(udir, rec.norm);
                }else{
                    dir = refract(udir, rec.norm, ri);
                }
            }

            r = Ray(rec.p, dir);
            vec3 emittedLight = rec.mat.emissionStr*rec.mat.col;
            incomingLight += emittedLight;
            col *= rec.mat.col;

        }else{
            break;
        }
    }
    float a = 0.5*(r.dir.y+1);
    vec3 bg  = mix(vec3(1.0), vec3(0.5, 0.7, 1.0), a);
    vec3 emittedLight = bg*1; // ambient light
    incomingLight += emittedLight*col;
    return incomingLight;
}

vec3 ray_dir(vec2 fragCoord) {
    vec3 topright = vec3(viewportWidth*cameraU);
    vec3 botleft = vec3(viewportHeight*cameraV);

    vec3 pixeldu = vec3(topright/screenW);
    vec3 pixeldv = vec3(botleft/screenH);

    vec3 topleft = cameraPos - focalLength*cameraW - topright/2 - botleft/2;

    vec3 pixel00 = topleft + 0.5*(pixeldu+pixeldv);

    vec2 jitter = rand_square(1.0);
    vec3 pixelCenter = pixel00 + (fragCoord.x + jitter.x) * pixeldu + (fragCoord.y + jitter.y) * pixeldv;

    vec3 dir = normalize(pixelCenter - cameraPos);
    return dir;
}

float linearToGamma(float linear){
    if(linear > 0){
        return sqrt(linear);
    }
    return 0.0;
}

vec3 gammaCorrect(vec3 linearCol){
    return vec3(linearToGamma(linearCol.x), linearToGamma(linearCol.y), linearToGamma(linearCol.z));
}

void main(){
    vec2 fragCoord = gl_FragCoord.xy;

    vec3 outCol = vec3(0,0,0);

    for(int i = 0; i < samplesPerPixel; i++){
        Ray currRay = Ray(cameraPos, ray_dir(fragCoord));
        outCol += ray_color(currRay);
    }
    outCol /= samplesPerPixel;

    finalColor = vec4(gammaCorrect(outCol), 1.0);
}
