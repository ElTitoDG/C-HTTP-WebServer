# Ruta del directorio a comprobar y crear si no existe
DIRECTORIO="build"

# Comprobar si el directorio existe
if [ ! -d "$DIRECTORIO" ]; then
  # Crear el directorio si no existe
  mkdir -p "$DIRECTORIO"
fi


cd build

cmake ..

make

./WebServer

cd ..
