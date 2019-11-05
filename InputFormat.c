#include <stdio.h>  
#include <stdlib.h> 
#include <sys/types.h>
#include <string.h>

#define MAX_LENGTH 21
//#define s_(x) #x
//#define s(x) s_(x)


int clean_stdin()
{
    while (getchar()!='\n');
    return 1;
}

//Lee y retorna un PID.
pid_t readPID(){
    
	int PID =0;  
    	
	int r;

	char enter;
    
	do
        {  
       		printf("Ingrese un PID, numero mayor a 0:\n");
        
		r=scanf("%9d%c",&PID,&enter);
	
		if(r==0 || enter !='\n' || PID<=0){

     		    printf("Error, la entrada no posee un formato valido.\n");

		}


   	} while (((r!=2 || enter!='\n') && clean_stdin()) || PID<=0);


	printf("Entrada exitosa, el PID es %d\n",PID);

	return  (pid_t) PID; 
}

//Lee y retorna un int.
int readInt( int min , int max ){
 
    	
	int r;
	
	int num;

	char enter;
    
	do
        {  
       		printf( "Ingrese un numero entre %d y %d:\n", min , max );
        
			r=scanf("%9d%c", &num , &enter);
	
			if( r == 0 || enter != '\n' || num < min || num > max ){

     		    printf("Error, la entrada no posee un formato valido.\n");

			}


   	} while ( ( (r != 2 || enter != '\n') && clean_stdin() ) || num < min || num > max);


	//printf("Entrada exitosa, el PID es %d\n",num);

	return num; 

}

//Lee y retorna un array
char * readCMD(){
	
	static char cmd[MAX_LENGTH];

	char * r;

    printf("Ingrese un comando:\n");

	do{
		
		r=fgets( cmd , MAX_LENGTH , stdin);
		
        if( r == NULL ){

            printf("El texto ingresado no posee el largo necesario.");

       }else
        {
            
            cmd[strcspn(cmd,"\n")];

        }
        

	}while(r==NULL);

	printf("%s",cmd);

	return cmd;
}



// //Lee y retorna un int.
// int readIP(){
 
    	
// 	int r;
	
// 	int num1;
//     int num2;
//     int num3;
//     int num4;

// 	char enter;
    
// 	do
//         {  
//        		printf( "Ingrese su IP:\n");
        
// 			r=scanf("%3d.%3d.%3d.%3d%c", &num1, &num2, &num3, &num4 , &enter);
	
// 			if( r == 0 || enter != '\n'){

//      		    printf("Error, la entrada no posee un formato valido.\n");

// 			}


//    	} while ( ( (r != 5 || enter != '\n') && clean_stdin() ));


// 	printf("Entrada exitosa, la IP es %d.%d.%d.%d\n", num1, num2, num3, num4);

// 	return num1; 

// }

int main(void)  
{ 

	printf("%d",readIP());

    return 0;  
}