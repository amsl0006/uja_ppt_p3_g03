/*******************************************************
Protocolos de Transporte
Grado en Ingeniería Telemática
Dpto. Ingeníería de Telecomunicación
Univerisdad de Jaén

Fichero: cliente.c
Versión: 1.0
Fecha: 23/09/2012
Descripción:
	Cliente de eco sencillo TCP.

Autor: Juan Carlos Cuevas Martínez

*******************************************************/
#include <stdio.h>
#include <winsock.h>
#include <time.h>
#include <conio.h>

#include "protocol.h"





int main(int *argc, char *argv[])
{
	SOCKET sockfd; //Crea el socket
	struct sockaddr_in server_in; //crea la estructura del socket y la añade al servidor
	char buffer_in[1024], buffer_out[1024],input[1024]; // Es el tamaño maximo de 1024 del buffer de los datos recibidos, enviados y los que entran.
	int recibidos=0,enviados=0; //Inicializacion de variables en 0
	int estado=S_HELO; //Primer valor en la variable estado ya que será una maquina de estados
	char option; //declaramos option

	//aqui definimos los parametros para el estado S_SUM
	char primera_palabra[4]="";
	char ultima_palabra[5]="";
	char entrada[80]="";
	unsigned int num1=0;
	unsigned int num2=0;


	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

    char ipdest[16]; //tamaño maximo para la ip de destino que se almacena en forma vectores
	char default_ip[16]="127.0.0.1";//tamaño maximo para la ip por defecto que la predefinimos.
	
	//Inicialización Windows sockets
	wVersionRequested=MAKEWORD(1,1);
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err!=0)
		return(0);

	if(LOBYTE(wsaData.wVersion)!=1||HIBYTE(wsaData.wVersion)!=1)
	{
		WSACleanup();
		return(0);
	}
	//Fin: Inicialización Windows sockets

	do{
		/* DECLARACION DE LA PRIMITIVA DE LOS SOCKET
		AF_INET: definido para ipv4
		SOCK_STREAM: para conexiones TCP
		0: se pone porque es por defecto
		*/
		sockfd=socket(AF_INET,SOCK_STREAM,0);

		if(sockfd==INVALID_SOCKET) // es un bucle para comprobar si se ha creado bien el socket si no lo hace bien imprime por pantalla error al crear socket y sale.
		{
			printf("CLIENTE> ERROR AL CREAR SOCKET\r\n");
			exit(-1);
		}
		else //Si crea bien el socket pide la ip de destino por pantalla(donde queremos acceder).
		{
			printf("CLIENTE> SOCKET CREADO CORRECTAMENTE\r\n");

		
			printf("CLIENTE> Introduzca la IP destino (pulsar enter para IP por defecto): ");
			gets(ipdest);//obtiene la ip de destino en la variable ipdest y almacena en la variable
			
			if(strcmp(ipdest,"")==0) // Compara con una cadena de caracteres vacia y al ser enter una cadena de caracteres vacia se cumple y copia la de default en la ipdest
				strcpy(ipdest,default_ip);


			server_in.sin_family=AF_INET;
			server_in.sin_port=htons(TCP_SERVICE_PORT);
			server_in.sin_addr.s_addr=inet_addr(ipdest);// es para si metes una ip de manera numerica unicamente
			
			estado=S_HELO; // Asigana estado a S_HELO
		
			//PRIMITIVA CONNECT
			// establece la conexion de transporte
			if(connect(sockfd,(struct sockaddr*)&server_in,sizeof(server_in))==0)
			{
				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n",ipdest,TCP_SERVICE_PORT);
			
		        
				recibidos=recv(sockfd,buffer_in,512,0); //recibimos la informacion del servidor para saber que esta listo


				//Inicio de la máquina de estados
				do{
					switch(estado)
					{
					case S_HELO:
						/*Identifica remitente.El servidor tendria que responder con un comando de repuesta
					que empiece por 2 para indicar que todo está correcto. Si
					la respuesta del servidor (almacenada en buffer_in), tiene como primer elemento
					algo distinto de 2, se volverá a pedir el nombre del host.*/
						
						
						printf("introduzca el nombre del host: "); //se introduce el nombre del host 
						gets(input);
						sprintf(buffer_out,"HELO %s%s",input,CRLF); //se recibe el mensaje de bienvenida para el host
						
						break;


					case S_MAIL_FROM://estado para la introduccion del remitente del correo electronico
						/*identifica al remitente.En este estado tambien comprobamos que la respuesta del servidor comienza por 2 para 
						que todo este correcto*/
						
						// establece la conexion de aplicacion 
						
						printf("CLIENTE> Escriba la direccion de correo electronico desde la cual va a enviar el mensaje: ");
						gets(input);
						if(strlen(input)==0) // si no se introduce ningun caracter se sale porque pasa al estado QUIT
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF); 
							estado=S_QUIT;
						}
						else{

						sprintf_s (buffer_out, sizeof(buffer_out), "%s%s%s",MA,input,CRLF); //aqui en vez de MA no valdria con solo input? y si queremos dejar MA, al principio, en el gets deberia de ser gets(MA) no?
						}
						//estado=S_RCPT_TO  // habria que poner si pasamos al siguiente estado o habria que dejarlo como esta?
						break;



					case S_RCPT_TO:// estado receptor donde se introduce el destinatario del correo
						/*Identifica al destinatario.En este estado tambien comprobamos la respuesta del servidor y 
						observamos que empiece por 2 para dar el comando SMTP como bueno.*/
						
						printf("CLIENTE> Introduzca la direccion del correo electronico a la que se enviara el mensaje: ");
						gets(input);
						if(strlen(input)==0)// si no se introduce ningun caracter se sale porque pasa al estado QUIT
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
						}
						else{

						sprintf_s (buffer_out, sizeof(buffer_out), "%s %s%s",RCPT,input,CRLF); //lo mismo que en el anterior con MA
						}	
						//estado=S_MENSAJE;
						break;
						


					case S_DATA://estado data que es para enviar datos
						/*Indica que el remitente está listo para la transferencia de líneas de texto, 
						cada linea de texto esta finalizada por CRL. La respuesta del servidor
						debe empezar por 3 para que el comando sea bueno.*/
						
						/*if(strlen(input)==0)
						{

							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
							
						}
						else{ //todo lo que se imprima por pantalla se envia al servidor
							*/
						sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",DATA,CRLF); //tenemos que introducir DATA seguido de un salto de linea, lo cual ya lo hacemos aqui asignando al segundo %s el valor de CRLF
							
						//printf("%s%s",input,CRLF);
						//sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",input,CRLF);
					
						
						break;
					


					case S_MENSAJE:
							/*Contiene la cabecera del mensaje y el mensaje en si. Para separar la cabecera de
							los datos escribimmos una línea en blanco y para indicar el fin de datos escribimos
							una línea que contiene ".". Solo recibimos respuesta del servidor cuando se finalizan
							los datos (cuando se escribe "."en una línea aparte).*/
					
						pritnf("fecha de origen:");
						gets(input);
						if(strlen(input)==0)
						{

							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
							
						}
						else{ //todo lo que se imprima por pantalla se envia al servidor
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",input,CRLF);
							}


						pritnf("asunto:"); //cambiar el orden de asunto en plan primero meter remitente y destinatario y despues introducir el mensaje
						gets(input);
						if(strlen(input)==0)
						{

							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
							
						}
						else{ //todo lo que se imprima por pantalla se envia al servidor
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",input,CRLF);
							enviados=send(sockfd,buffer_out,(int)strlen(buffer_out),0);

							sprintf_s (buffer_out, sizeof(buffer_out), "%s",CRLF);
						}

						prinf("mensaje de correo: \r\n");
						//para poder escribir varias lineas dentro del mensaje hacemos...

						do{
							gets(input);
						if(strlen(input)==0)
						{

							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
							
						}
						else{ //todo lo que se imprima por pantalla se envia al servidor
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",input,CRLF);
							
						}




						pritnf("destinatario:");
						gets(input);
						if(strlen(input)==0)
						{

							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
							
						}
						else{ //todo lo que se imprima por pantalla se envia al servidor
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",input,CRLF);
							
							}


						pritnf("remitente:");
						gets(input);
						if(strlen(input)==0)
						{

							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
							
						}
						else{ //todo lo que se imprima por pantalla se envia al servidor
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",input,CRLF);
							
							}

					

					/*Introducimos una opción para el usuario por si quiere escribir un nuevo correo sin
					cerrar la sesion que ya esta iniciada. Si selecciona volver a enviar un mensaje, volveremos
					al estado 2, MAIL_FROM (aunque indiquemos S_HELO, al salir del caso con el
					break, se incrementa en 1 el estado). Si no quiere escribirlo pasamos al ultimo caso.*/
					printf(" ¿quiere enviar otro mensaje antes de cerrar sesion?(s/n)");
					if(getch()=='s')
						estado=S_HELO;

					break;

					case S_QUIT:
					/*estado para Cerrar conexión, que respondera con un comando que comienza por 2, si no es así, se volverá
					a ejecutar este comando hasta que el mensaje nos indique que todo está correcto.*/
					do{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s",CRLF);
						enviados=send(sockfd,buffer_out,(int)strlen(buffer_out),0);
						recibidos=recv(sockfd,buffer_in,512,0);	
						printf("%s",buffer_in);
						if(buffer_in[0]!='2')
							printf("codigo de respuesta no valido, intentelo de nuevo");
					}while(buffer[0]!='2');
				break;

				}
				   

				//**************************************************************************************************************************
					
					//Envio
				//	if(estado!=S_HELO){
					// Ejercicio: Comprobar el estado de envio
						// La primitiva SEND se utiliza para el envio de datos y en este caso esta compuesta por:
							//sockfd: Socket para recibir y enviar
							// buffer_out: puntero que apunta al mensaje que se quiere enviar.
							//(int)strlen(buffer_out): Tamaño en bytes del mensaje que se va a enviar.
							//0: es como va la llamada.

						enviados=send(sockfd,buffer_out,(int)strlen(buffer_out),0);
					//-*-*-*-*-*-*-*-*-*-*-*-*--*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
					/* vamos ha realizar una comprobacion de los datos enviados muy parecida a lo que se hace en recibidos*/
					if(enviados<=0){// si los datos enviados tienen un valor de menor o igual que 0
						DWORD error=GetLastError();
						if(enviados<0){ // si los datos enviados tienen un valor de -1 hay error
							printf("SERVIDOR> Error %d en el envio de datos\r\n", error);
							estado=S_QUIT;
							continue;
						} 
						else{ // si no, es decir, si su valor es 0 se cierra la conexion
							printf("SERVIDOR> Conexión con el cliente cerrada\r\n");
							estado=S_QUIT;
							continue;
						}
					}
					else{ // el valor es mayor que 0 y por tanto los datos se envian correctamente
						printf("SERVIDOR> Datos enviados correctamente\r\n");}
				//}
				
				//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
					
					
					
					
					//Recibo
				
					//La primitiva RECV se utiliza para recibir datos:
						//sockfd : Socket que se usa para enviar o recibir.
						//buffer_in: puntero que apunta al mensaje que se quiere recibir.
						//512: Tamaño en bytes del mensaje que recibimos
						//0: es como va la llamada.
					
					recibidos=recv(sockfd,buffer_in,512,0);

					if(recibidos <=0){
						DWORD error=GetLastError();
						if(recibidos<0)
						{
							printf("CLIENTE> Error %d en la recepción de datos\r\n",error);
							estado=S_QUIT;
							continue;
						}
						else
						{
							printf("CLIENTE> Conexión con el servidor cerrada\r\n");
							estado=S_QUIT;
							continue;
						
					
						}
					}else
					{
						buffer_in[recibidos]=0x00;
						printf(buffer_in);
						//if(estado!=S_DATA && strncmp(buffer_in,OK,2)==0) // si el estado es diferente de DATA y comparamos los dos primeros caracteres del mensaje recibido y OK y si coinciden pasa al siguiente estado.
							//estado++;
						switch (estado){
						case S_HELO:
							if (buffer_in[0]=='2'){
								estado++;}
							else estado=S_QUIT;
						break;
						case S_MAIL_FROM:
							if (buffer_in[0]=='2'){
								estado++;}
							else estado=S_QUIT;
							break;
						case S_RCPT_TO:
							if (buffer_in[0]=='2'){
								estado++;}
							else estado=S_QUIT;//HABRIA QUE VOLVER AL MAIL FROM? O SEGUIRIAMOS AQUI?
							break;
						case S_DATA:
							if (buffer_in[0]=='3'){
								estado++;}
							else estado=S_QUIT;
							break;
						case S_MENSAJE:
							if (buffer_in[0]=='2'){
							printf("¿quiere mandar otro mensaje antes de finalizar sesion?(s/n)");
							if(getch()=='s'){
								estado=S_MAIL_FROM;}
							else{ estado=S_QUIT;}
							break;
							
						
						}
					}
					
				}
			}while(estado!=S_QUIT);
			
			//while(estado!=S_QUIT);
				
	
		
			}
			else
			{
				printf("CLIENTE> ERROR AL CONECTAR CON %s:%d\r\n",ipdest,TCP_SERVICE_PORT);
			}		
			// fin de la conexion de transporte
			closesocket(sockfd);//cerramos el socket
			
		}	
		printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
		option=_getche();

	}while(option!='n' && option!='N');

	
	
	return(0);

}
