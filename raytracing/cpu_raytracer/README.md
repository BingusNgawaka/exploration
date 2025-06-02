# CPU Raytracer following Ray Tracing in One Weekend
[Ray Tracing In One Weekend](https://raytracing.github.io/books/RayTracingInOneWeekend.html)

## Requirements
c++ and cmake.

## Compiling and Running
in this dir make a build folder then cd into it, cmake .., make, then run the main exec produced.
```bash
mkdir build
cd build
cmake ..
make
./main
```

## Rambling and Afterthoughts
This free online book was recommended to me in a DSA tutorial by my tutor Euan. Had a glance through and knew I had to do this. Note that this took a lot longer than a weekend with school and work lol.
While I basically just followed along one for one with the book in terms of code, I still learned a lot and had tonnes of fun although that leaves me with very little to discuss here as anything I say is already said significantly better in the book.

I think the most fun I had was the first little bit with a single simple sphere with no fancy effects and only one ray per pixel.
My delusional ass thought that I could maybe squeeze out like 5-10fps from this raytracer after all was said but I was struggling to get 3fps with a single sphere and 1 ray per pixel.

I spent quite a while trying to optimize too (eventually giving up and rendering a single frame on startup) and that was quite fun, I implemented this random number generator that I saw in a video about mario 64 (it's just a simple xor shift) since c++'s stdl random num generator is really slow (idk if i was doing it wrong though).

I used Raylib too (due to my delusions of realish time performance) to render everything and that was fun setting up too. Found a great video about efficiently editing and rendering individual pixels [here](https://www.youtube.com/watch?v=xDOYyBl7S-g).
At the end though it probably would've been better to leave it as a CLI as it ended up just being annoying to work with.

Raylib has support for shaders too though so I will probably try using it when I move everything over to the GPU.

That's my next goal at least, moving everything over to the GPU. I did a bit of graphics programming a year ago with OPENGL, GLEW, and GLUT but I had a very surface level understanding of it so diving back into shaders for more complex cases will be fun.

I am worried Raylib won't be super amazing but I guess we'll have to see, if it's not then I probably will end up migrating to raw OPENGL.

I was super inspired by Sebastian Lague too and his [video on BVH's](https://www.youtube.com/watch?v=C1H4zIiCOaI), I mean most of everything I do is inspired by a video of his as well as writing my afterthoughts/methods down here.

Not a lot to say really, it's all just linear algebra lol. I had a pretty good grasp on things to begin with so mainly putting it into practices is where the learning happened.

Great book, 8/10. Wish it went into proofs/details a lil more, but apparently his next two books are much more in depth so will definitely be going through those.
