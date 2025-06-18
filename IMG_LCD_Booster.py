print('''
Asegurate de tener las siguientes librerias
matplotlib
numpy''')

from matplotlib import image, pyplot
import numpy
import os


while True:
    path = input('''
Ingresa el path de tu archivo (.jpg, .jpeg)
''')
    if os.path.exists(path):
        break
    else:
        print("No es valido, vuelve a intentar")
        print('''
Te recomiendo dejar el archivo en la misma carpeta donde estás corriendo este archivo, o dejarlo
en el mismo workspace de Visual Studio Code que tengas abierto (en caso de que lo estes usando).
''')
print("Gracias, seguimos")

img = image.imread(path)

img_128 = numpy.zeros((128, 128, 3))

y_img, x_img, z = img.shape

delta_x = 0
delta_y = 0
smallest = min(y_img, x_img)

size = int(smallest/128)-1
while True:
    try:
        for j in range(delta_y + size, delta_y + size + 128):
            for i in range(delta_x + size, delta_x + size + 128):
                for n in range(3):
                    if i + size * ((i - size) + 1) > x_img or j + size * ((j - size) + 1) > y_img:
                        valor = 0
                    else:
                        mask = img[j + size * ((j - size) - 1) : j + size * ((j - size) + 1), i + size * ((i - size) - 1) : i + size * ((i - size) + 1), n]
                        valor = numpy.average(mask)


                    img_128[j - size - delta_y, i - size - delta_x, n] = int(valor)/255
        pyplot.imshow(img_128)
        pyplot.show()
    except ValueError or RuntimeWarning:
        print("El valor de cambio no es valido, por favor ingresa otro.")



    valor = -1
    seguir = False
    while True:
        valor = input('''
Deseas desplazar la imagen para cambiar el resultado?
        0 si quedo bien
        1 si quieres cambiar
        ''')
        if valor == '1':
            delta_x = int(input('''
Ingresa el valor de desplazamiento en x (>0) (de la imagen original)        
'''))
            delta_y = int(input('''
Ingresa el valor de desplazamiento en y (>0)        
'''))   
            break
        elif valor == '0':
            seguir = True
            break
        else:
            print(" Valor no válido, intenta nuevamente")
    if seguir:
        break


img_565 = numpy.zeros((128, 128, 3))
y, x, z = img_565.shape
for n in range(3):
    for i in range(x):
        for j in range(y):
            if n != 1:
                img_565[j, i, n] = int(img_128[j, i, n] * 31)
            else:
                img_565[j, i, n] = int(img_128[j, i, n] * 63)


img_out = numpy.zeros((y, x))

for n in range(3):
    if n == 2:
        exp = 11
    elif n == 1:
        exp = 5
    else:
        exp = 0
    for i in range(x):
        for j in range(y):
            img_out[j, i] += int(img_565[j, i, n] * (2**exp))

opcion = input('''
Opciones de entrega:
Archivo de texto (.txt) (0)
Texto en consola (cualquier otro caracter)
''')
txt = ''
for j in range(y):
        txt += "{"
        for i in range(x):
            if i != 127:
                txt += f"{int(img_out[j, i]):#06x}, "
            else:
                txt += f"{int(img_out[j, i]):#06x}"
        if j != 127:
            txt += "}, \n"
        else:
            txt += "}"

if opcion == '0':
    
    with open("img_Vitis.txt", "w") as archivo:
        archivo.write(txt)
    print("Archivo creado: img_Vitis.txt")
else:    
    print("Listo, ahora copia el siguiente texto, y listo para implementar en Vitis")
    for j in range(y):
        print("{", end = '')
        for i in range(x):
            if i != 127:
                print(f"{int(img_out[j, i]):#06x}, ", end = '')
            else:
                print(f"{int(img_out[j, i]):#06x}", end = '')
        if j != 127:
            print("},")
        else:
            print("}")

input('''
Hasta luego! 
      
Creado por Samuel Rodríguez para IEE2463 - SEP
Presiona enter para terminar
''')