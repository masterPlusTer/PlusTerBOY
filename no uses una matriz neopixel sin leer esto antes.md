hay algo de lo que no se habla y que al momento de programar da muchos dolores de cabeza, 
y es que las matrices neopixel son muy diferentes a las matrices led de toda la vida, 
la matriz neopixel esta configurada como una tira led. 
y si no tienes esto en cuenta, te vas a encontrar con que al momento de ejecutar tu codigo cosas raras pasen cuando te muevas en el eje Y
para resolver esto tendras que, o bien hacerte de una libreria que se ocupe de esto y entonces te despreocupes y codifiques como si estuvieras usando una matriz led normal
o bien ocuparte de ir fila de por medio invirtiendo el orden de los pixeles
porque la matriz neopixel hace literalmente esto:


                               
            ![image](https://github.com/user-attachments/assets/cef5f99f-48fb-4f93-bf46-ae930de865d6)
                        

si te fijas bien, va como una serpiente de izquierda a derecha y luego de derecha a izquierda, porque la matriz neopixel es una tira led doblada basicamente,
a diferencia de la matriz normal donde todas las filas comienzan en la izquierda 

por esta razon lo que hay que hacer es, invertir la orientacion de las filas impares (recuerda que empezamos por el cero , asi que hay que invertir la 1, 3, 5, 7, 9 etc etc )



ya se que es una cosa muy basica, pero justamente porque es basico todos lo damos por hecho, nadie lo comenta, nadie lo aclara, y por consecuencia todos lo olvidamos
