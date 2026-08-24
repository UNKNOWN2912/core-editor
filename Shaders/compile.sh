cd "$(dirname "$0")" 
echo "Compiling shaders"
./compile_shader.sh skybox
./compile_shader.sh fullscreen
./compile_shader.sh shadow
./compile_shader.sh directional
./compile_shader.sh physical
./compile_shader.sh text
./compile_shader.sh bezier
./compile_shader.sh debugLine
./compile_shader.sh prepass
