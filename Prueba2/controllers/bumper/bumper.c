/*Codigo creado por Rebeca Abigail Mendoza Farias 
Programación de Robots
Proyecto Tercer Parcial 
*/

//Las bibliotecas del código
#include <webots/motor.h>
#include <webots/robot.h>
#include <webots/touch_sensor.h>
#include <webots/Camera.h>
#include <stdio.h>
#include <webots/gps.h>

/*Se obtienen los colores ANSI para uso de la cámara
En este caso solo utilizo el rojo, verde y azul, también es importante
el RESET que elimina el color.
*/
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_BLACK "\x1b[30m"

//Defino la velocidad del robot y el TIME_STEP
#define SPEED 6.5
#define TIME_STEP 64

//Estas variables son para elegir el tipo de color
enum BLOB_TYPE { RED, GREEN, BLUE, NONE };

//Variables del GPS
typedef enum { GPS, SUPERVISED } gps_mode_types;

//Ciclo inicial
int main() {
  //Inicializamos unos dispositivos
  //Sensor de tacto
  WbDeviceTag bumper;
  //Motor de las ruedas
  WbDeviceTag left_motor, right_motor;
  //Camara
  WbDeviceTag camera;

  //El width y height son para la cámara
  int width, height;
  //Este Time wait es para que espere los primeros 5 segundos en la linea del ring
  int TIME_WAIT = 50;

  //Variables de color tipo entero
  int red, blue, green;

  //Procedemos a hacer dos listas, una del nombre de los colores y otra con sus ansi respectivos
  const char *color_names[3] = {"red", "green", "blue"};
  const char *ansi_colors[3] = {ANSI_COLOR_RED, ANSI_COLOR_GREEN, ANSI_COLOR_BLUE};
  //Current blob sirve para saber cual es el color actual 
  enum BLOB_TYPE current_blob;
  
  //Esta variable se utiliza para el sensor de tacto, si es 0 significa que no ha detectado nada
  int movement_counter = 0;
  //Velocidad del motor
  double left_speed, right_speed;
  //Variables para un for que usaremos en el ciclo inicial
  int i;
  int j;
  
  //inicializamos el robot
  wb_robot_init();
  
  //Obtenemos el GPS 
  WbDeviceTag gps = wb_robot_get_device("gps");
  wb_gps_enable(gps, TIME_STEP);
  //Obtenemos el sensor de Tacto
  bumper = wb_robot_get_device("bumper");
  wb_touch_sensor_enable(bumper, TIME_STEP);
  //Obtenemos el valor de nuestros motores y inicializamos velocidad y posición
  left_motor = wb_robot_get_device("left wheel motor");
  right_motor = wb_robot_get_device("right wheel motor");
  wb_motor_set_position(left_motor, INFINITY);
  wb_motor_set_position(right_motor, INFINITY);
  wb_motor_set_velocity(left_motor, 0.0);
  wb_motor_set_velocity(right_motor, 0.0);
  //Obtenemos la cámara de los robots
  camera = wb_robot_get_device("CAM");
  wb_camera_enable(camera, TIME_STEP);
  width = wb_camera_get_width(camera);
  height = wb_camera_get_height(camera);
  //Habilitamos el teclado para el uso de nuestro GPS
  wb_keyboard_enable(TIME_STEP);
  //presionando G se obtiene el GPS
  printf("Press 'G' to read the GPS device's position\n");
  
  /* control loop */
  while (wb_robot_step(TIME_STEP) != -1 ) {
  
  //Condicional para imprimir la posición actual de ambos robots
  if(wb_keyboard_get_key()== 'G'){
     const double *gps_values = wb_gps_get_values(gps);
     printf("GPS position: %.3f %.3f %.3f\n", gps_values[0], gps_values[1], gps_values[2]);
  }
    //reset a la variable del sensor de tacto 
    movement_counter=0;
    //Pregunta si ha detectado algo el sensor de tacto
    if (wb_touch_sensor_get_value(bumper) > 0)
      movement_counter = 15;
    //reset de las variables de color 
    red = 0;
    green = 0;
    blue = 0;

    //Obtenemos la imagen de la cámara
    const unsigned char *image = wb_camera_get_image(camera);
    
    //Condicional principal en la que elegimos la velocidad o movimientos del robot
    if (image) { 
    //El robot inicia con una velocidad estándar
    left_speed = SPEED/5;
    right_speed = SPEED/5;

      /*Este ciclo es muy particular, son dos ciclos aninados en el cual mendiante la imagen obtenida de la cámara
      mide la saturación de color, y va incrementando la variable de tipo entero para saber que color
      tiene más saturacvión en ese mmento*/
      for (i = width / 3; i < 2 * width / 3; i++) {
        for (j = height / 2; j < 3 * height / 4; j++) {
          red += wb_camera_image_get_red(image, width, i, j);
          blue += wb_camera_image_get_blue(image, width, i, j);
          green += wb_camera_image_get_green(image, width, i, j);
        }
      }

      /*Con los valores de arriba se hacen 3 condicionales principales, ya que con los valores de los colores
      se puede saber que color estaba saturado y se le guarda en el current_blob*/
      if ((red > 3 * green) && (red > 3 * blue))
        current_blob = RED;
      else if ((green > 3 * red) && (green > 3 * blue))
        current_blob = GREEN;
      else if ((blue > 3 * red) && (blue > 3 * green))
        current_blob = BLUE;
      else
        current_blob = NONE;
        
        /*En estos condicionales se puede saber en que parte del ring está el robot y si está tocando o no al otro robot
        */
        if(current_blob == GREEN && TIME_WAIT > 0){//Se para en la linea verde
          left_speed = 0;
          right_speed = 0;
          TIME_WAIT --;
          printf("Hola1");
        }else if(movement_counter > 0){//Si detecta algo, empuja a toda velocidad
          left_speed = SPEED;
          right_speed = SPEED;
        }/*Si no está tocando nada pero se encuentra dentro del ring tiene una velocidad normal y va recto*/
        else if((current_blob == BLUE || current_blob == GREEN) && movement_counter == 0){
          left_speed = SPEED/5;
          right_speed = SPEED/5;
        }/*Si está siendo empujado y detecta el final del ring intentará esquivar*/
        else if (current_blob == RED && movement_counter > 0){
          left_speed = -SPEED/5;
          right_speed = SPEED/5;
        }/*Si se encuentra en el final del ring y no detecta nada (Ya tiro al otro robot), se para*/
        else if(current_blob == RED && movement_counter == 0){
          left_speed = 0;
          right_speed = 0;
        }
        else{/*velocidad normal*/
          left_speed = SPEED/5;
          right_speed = SPEED/5;
        }
    }
    //Se pasa la velocidad a los motores 
    wb_motor_set_velocity(left_motor, left_speed);
    wb_motor_set_velocity(right_motor, right_speed);
  }
  //se limpian variables
  wb_robot_cleanup();

  return 0;
  //fin del código
}


