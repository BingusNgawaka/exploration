#include "game.h"
#include <cmath>
#include <limits>
#include <memory>
#include <raylib.h>
#include <algorithm>

// consts
const double inf = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;
uint64_t state {0xCAFEBABE12345678ULL};

uint64_t xorshift64(uint64_t& state){
    state ^= state >> 12;
    state ^= state << 25;
    state ^= state >> 27;
    return state * 2685821657736338717ULL;
}

double fast_rand_double(){
    return xorshift64(state) / static_cast<double>(UINT64_MAX + 1.0);
}

double fast_rand_double(double min, double max){
    return min + (max-min)*fast_rand_double();
}

double linear_to_gamma(double lin){ // values from 0-255 cause my dumbass went with that
    if(lin > 0){
        return std::sqrt(lin/255.0)*255;
    }
    return 0;
}

struct Interval{
    double max, min;

    Interval(): min(+inf), max(-inf){}

    Interval(double min, double max): min(min), max(max) {}

    double size() const{
        return max-min;
    }

    // inclusive of bounds: p is in [min, max]
    bool contains(double point){
        return point >= min && point <= max;
    }

    // exclusive of bounds: p is in (min, max)
    bool surrounds(double point){
        return point > min && point < max;
    }

    static const Interval empty, universe; // handy static insts
};

const Interval empty {};
const Interval universe {-inf, +inf};

struct Vec3 {
    double x;
    double y;
    double z;

    Vec3(): x(0), y(0), z(0){}
    Vec3(double x, double y, double z): x(x), y(y), z(z){}

    static Vec3 random(){
        return Vec3(fast_rand_double(), fast_rand_double(), fast_rand_double());
    }

    static Vec3 random(double min, double max){
        return Vec3(fast_rand_double(min,max), fast_rand_double(min,max), fast_rand_double(min,max));
    }

    double operator[](int i) const {
        if(i == 0){
            return x;
        }else if(i == 1){
            return y;
        }else if(i == 2){
            return z;
        }else{
            std::cerr << "oops, vec3 out of index! owo";
            return 0;
        }
    }

    const Vec3 operator-() const {
        return Vec3(-x, -y, -z);
    }

    const Vec3 operator-(const Vec3& other) const{
        return Vec3(x-other.x, y-other.y, z-other.z);
    }

    const Vec3 operator+(const Vec3& other) const{
        return Vec3(x+other.x, y+other.y, z+other.z);
    }

    const Vec3 operator*(const double t){
        return Vec3(x*t, y*t, z*t);
    }

    const Vec3 operator*(const Vec3& other){
        return Vec3(x*other.x, y*other.y, z*other.z);
    }

    Vec3 operator/(const double t){
        return Vec3(x/t, y/t, z/t);
    }

    Vec3 operator=(const Vec3& other){
        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
    }

    Vec3 operator+=(const Vec3& other){
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Vec3 operator-=(const Vec3& other){
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    Vec3 operator*=(const double t){
        x *= t;
        y *= t;
        z *= t;
        return *this;
    }

    Vec3 operator/=(const double t){
        x /= t;
        y /= t;
        z /= t;
        return *this;
    }

    const double magSqrd() const{
        return x*x+y*y+z*z;
    }

    const double mag() const{
        return std::sqrt(magSqrd());
    }

    const Vec3 normalized() const{
        double n = 1/mag();
        return Vec3(x*n,y*n,z*n);
    }

    const bool near_zero() const{
        double epsilon {1e-8};
        return (std::fabs(x)<epsilon && std::fabs(y)<epsilon && std::fabs(z)<epsilon);
    }
};
const Vec3 operator*(double t, const Vec3& v){
    return Vec3(t*v.x, t*v.y, t*v.z);
}
const std::ostream& operator<<(std::ostream& out, const Vec3& v){
    return out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}
const double dot(const Vec3& v1, const Vec3& v2){
    return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
}

// get a rand vec that lies on the unit sphere
Vec3 rand_unit_vec(){
    while(true){
        Vec3 v {Vec3::random(-1, 1)};
        double lenSqr {v.magSqrd()};
        if(lenSqr <= 1 && lenSqr > 1e-160){ // floating point black hole near center
            return v.normalized();
        }
    }
}

Vec3 rand_vec_on_hemisphere(const Vec3& norm){
    Vec3 vec_on_sphere {rand_unit_vec()};
    if(dot(vec_on_sphere, norm) > 0){
        // lies within the correct hemisphere
        return vec_on_sphere;
    }
    return -vec_on_sphere;
}

Vec3 reflect(const Vec3& v, const Vec3& n){
    return v - 2*dot(v,n)*n;
}

// this shi just snells law
Vec3 refract(const Vec3& v, const Vec3& n, double etai_over_etat){
    double cos_theta {std::fmin(dot(-v, n), 1.0)}; // clamped to 1
    Vec3 r_out_perp {etai_over_etat*(v+cos_theta*n)};
    Vec3 r_out_par {-std::sqrt(std::fabs(1.0-r_out_perp.magSqrd())) * n};
    return r_out_perp + r_out_par;
}

struct ray{
    Vec3 orig;
    Vec3 direction;
    double dirSqrd;
    double dirSqrdInv;

    ray(){}
    ray(const Vec3& origin, const Vec3& dir): orig(origin), direction(dir), dirSqrd(dot(dir, dir)), dirSqrdInv(1.0/dirSqrd){}

    const Vec3& dir() const{
        return direction;
    }

    Vec3 at(double t) const{
        return orig + (t*direction);
    }
};

struct material; // HitRecord needs a ref to material and vice versa

struct HitRecord{
    Vec3 p;
    Vec3 norm;
    double t;
    bool front_face;

    std::shared_ptr<material> mat;

    void set_face_norm(const ray& r, const Vec3& out_norm){
        front_face = dot(r.dir(), out_norm) < 0;
        norm = front_face ? out_norm : -out_norm;
    }
};
struct Hittable{
    virtual bool hit(const ray& r, Interval t_range, HitRecord& rec) const = 0;
    virtual ~Hittable() = default;
};


struct material{
    virtual ~material() = default;

    virtual bool scatter(const ray& r_in, const HitRecord& rec, Vec3& atten, ray& scattered) const {
        return false;
    }
};

struct lambertian : public material { // basic diffuse like our first test sphere
    Vec3 albedo; // latin for whiteness

    lambertian(const Vec3& albedo): albedo(albedo){}

    virtual bool scatter(const ray& r_in, const HitRecord& rec, Vec3& atten, ray& scattered) const override{
        Vec3 scatter_dir {rec.norm + rand_unit_vec()};
        if(scatter_dir.near_zero()){
            scatter_dir = rec.norm;
        }
        scattered = ray(rec.p, scatter_dir);
        atten = albedo;
        return true;
    }
};

struct metal : public material { 
    Vec3 albedo; // latin for whiteness
    double fuzz; // always < 1

    metal(const Vec3& albedo, double fuzz): albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    virtual bool scatter(const ray& r_in, const HitRecord& rec, Vec3& atten, ray& scattered) const override{
        Vec3 reflected_dir {reflect(r_in.dir(), rec.norm)};
        reflected_dir = reflected_dir.normalized() + fuzz*rand_unit_vec();
        scattered = ray(rec.p, reflected_dir);
        atten = albedo;
        return (dot(scattered.dir(), rec.norm) > 0);
    }
};

struct dielectric : public material { 
    double refraction_index;

    dielectric(double refraction_index): refraction_index(refraction_index){}

    static double reflectance(double cos_theta, double ri){
        // schlicks approx for reflectance (varying reflectivity with angle)
        double r0 = (1-ri)/(1+ri);
        r0 = r0*r0;
        return r0 + (1-r0)*std::pow((1-cos_theta),5);
    }

    virtual bool scatter(const ray& r_in, const HitRecord& rec, Vec3& atten, ray& scattered) const override{
        atten = Vec3(1.0,1.0,1.0);
        double ri {rec.front_face ? (1.0/refraction_index) : refraction_index};

        Vec3 u_dir {r_in.dir().normalized()};

        double cos_theta {std::fmin(dot(-u_dir, rec.norm), 1.0)}; // clamped to 1
        double sin_theta {std::sqrt(1.0 - cos_theta*cos_theta)};
        Vec3 dir;
        bool cannot_refract {ri * sin_theta > 1.0};
        if(cannot_refract || reflectance(cos_theta, ri) > fast_rand_double()){
            // must reflect
            dir = reflect(u_dir, rec.norm);
        }else{
            // can refract
            dir = refract(u_dir, rec.norm, ri);
        }

        scattered = ray(rec.p, dir);
        return true;
    }
};

struct Sphere : public Hittable{
    Vec3 center;
    double radius;
    double radiusSqrd;
    double radiusInv;
    std::shared_ptr<material> mat;

    Sphere(Vec3 center, double r, std::shared_ptr<material> mat):
        center(center), radius(r), radiusSqrd(r*r), radiusInv(1.0/r), mat(mat)
    {}

    bool hit(const ray& r, Interval t_range, HitRecord& rec) const override {
        Vec3 oc = center - r.orig;
        double h {dot(r.dir(), oc)};
        double c {dot(oc,oc) - radiusSqrd};
        double disc {h*h - r.dirSqrd*c};
        if(disc < 0){
            return false;
        }

        double sqrtDisc {std::sqrt(disc)};
        double t {(h-sqrtDisc)*r.dirSqrdInv};
        if(!t_range.surrounds(t)){
            // double check other possible root if - is invalid
            t = (h+sqrtDisc)*r.dirSqrdInv;
            if(!t_range.surrounds(t)){
                return false;
            }
        }

        rec.t = t;
        rec.p = r.orig + t*r.dir();
        rec.set_face_norm(r, radiusInv*(rec.p - center));
        rec.mat = mat;

        return true;
    }
};

class Screen : public Entity{
    private:
        int image_width;
        int image_height;

        double viewport_width;
        double viewport_height;

        Vec3 camera_center {};
        Vec3 pixel00 {};
        Vec3 pixel_du {};
        Vec3 pixel_dv {};
        double focal_length {1};

        PixelTexture pixels;
        Game& game_ref;

        std::vector<std::shared_ptr<Hittable>> hittables;

        int samples_per_pixel {100};
        double blur_radius {1};

        int max_depth {50};
        int curr_depth {0};

        double samples_scale {1.0/samples_per_pixel};

    public:
        Screen(Game& game, double viewport_height, int image_width, double aspect_ratio):
            game_ref(game), image_width(image_width), image_height(image_width/aspect_ratio), pixels(image_width, image_height)
        {
            viewport_height = viewport_height;
            viewport_width = viewport_height*(static_cast<double>(game.windowW)/game.windowH);
            
            // right hand coord system
            Vec3 viewport_u {viewport_width, 0, 0}; // top right
            Vec3 viewport_v {0, -viewport_height, 0}; // bot left

            pixel_du = viewport_u/image_width;
            pixel_dv = viewport_v/image_height;

            Vec3 viewport_top_left {camera_center - Vec3(0, 0, focal_length) - viewport_u/2 - viewport_v/2};
            pixel00 = viewport_top_left + 0.5*(pixel_du+pixel_dv);
        }

        void clear(){
            hittables.clear();
        }

        void add_hittable(std::shared_ptr<Hittable> entity){
            hittables.push_back(entity);
        }

        void remove_hittable(std::shared_ptr<Hittable> entity){
            auto it = std::find(hittables.begin(), hittables.end(), entity);
            if(it != hittables.end()){
                hittables.erase(it);
            }
        }

        bool get_world_hit(const ray& r, HitRecord& rec){
            HitRecord temp_rec;
            bool hit_anything {false};
            double closest = inf;
            double ray_tmin {0.001}; // not 0 to avoid shadow acne

            for(const auto& entity : hittables){
                if(entity->hit(r, Interval(ray_tmin, closest), temp_rec)){
                    hit_anything = true;
                    closest = temp_rec.t;
                    rec = temp_rec;
                }
            }

            return hit_anything;
        }

        Vec3 get_ray_color(const ray& r){
            if(curr_depth >= max_depth){
                return Vec3();
            }
            ++curr_depth;

            HitRecord rec;
            if(get_world_hit(r, rec)){
                ray scattered;
                Vec3 atten;
                if(rec.mat->scatter(r, rec, atten, scattered)){
                    return atten*get_ray_color(scattered);
                }
                return Vec3();
            }
            Vec3 unit_dir {r.dir().normalized()};
            float a = 0.5*(unit_dir.y + 1.0);
            float a_inv = 1.0 - a;
            return Vec3(a_inv*255 + a*128,a_inv*255 + a*180,(a_inv+a)*255);
        }

        Vec2 getRandBox(){
            return Vec2((fast_rand_double()-0.5)*blur_radius, (fast_rand_double()-0.5)*blur_radius);
        }

        ray get_ray(int i, int j){
            Vec2 offset {getRandBox()};
            Vec3 pixel_center { pixel00 + ((i+offset.x)*pixel_du) + ((j+offset.y)*pixel_dv) };
            Vec3 ray_dir { pixel_center - camera_center };
            ray r {camera_center, ray_dir};
            return r;
        }

        void update_screen(){
            for(int i = 0; i < image_width; ++i){
                for(int j = 0; j < image_height; ++j){

                    Vec3 col {0,0,0};

                    for(int k = 0; k < samples_per_pixel; ++k){
                        curr_depth = 0;
                        ray r {get_ray(i, j)};

                        col += get_ray_color(r);
                    }

                    col *= samples_scale;

                    pixels.setPixel(i, j, linear_to_gamma(col.x), linear_to_gamma(col.y), linear_to_gamma(col.z), 255);
                }
            }
            pixels.setTexture();
        }

        void update(float dt) override{
        }

        void draw() override{
            pixels.draw(0,0,game_ref.windowW,game_ref.windowH);
            DrawFPS(10, 10);
        }
};

int main(){
    int upscale {2};
    Game game {400*upscale, 225*upscale, "uwu"}; // w, h, title
    std::unique_ptr<Screen> scr {std::make_unique<Screen>(game, 2.0, 400, 16.0/9.0)};

    std::shared_ptr<material> ground {std::make_shared<lambertian>(Vec3(0.8, 0.8, 0.0))};
    std::shared_ptr<material> mat1 {std::make_shared<lambertian>(Vec3(0.1, 0.2, 0.5))};
    std::shared_ptr<material> mat2 {std::make_shared<dielectric>(1.50)};
    std::shared_ptr<material> bubble {std::make_shared<dielectric>(1.0/1.50)};
    std::shared_ptr<material> mat3 {std::make_shared<metal>(Vec3(0.8, 0.6, 0.2), 0.3)};

    scr->add_hittable(std::make_shared<Sphere>(Vec3(0,-100.5,-1), 100, ground));

    scr->add_hittable(std::make_shared<Sphere>(Vec3(-0.5,0,-1), 0.5, mat2));
    scr->add_hittable(std::make_shared<Sphere>(Vec3(-0.5,0,-1), 0.4, bubble));
    scr->add_hittable(std::make_shared<Sphere>(Vec3(0.5,0,-1), 0.5, mat3));


    scr->update_screen(); // performance getting way too slow for real time lol just do once at start
    game.add_entity(std::move(scr));

    game.main();
    return 0;
}
