/*
Analizador Léxico Práctica de Programación Nro. 1
Curso: Compiladores y Lenguajes de Bajo de Nivel

Descripcion:
Implementa un analizador léxico que reconoce números, identificadores, 
palabras reservadas, operadores y signos de puntuación para un lenguaje
con sintaxis tipo JSON.
*/

#include "anlex.h"
//Variables Globales
int consumir;
char cad[5*TAMLEX];
token t;
int error = 0;

//Variables para el analizador léxico
FILE *fuente;            // Archivo de entrada (fuente)
FILE *output;            // Archivo de salida
char buff[2*TAMBUFF];	// Buffer para lectura de archivo fuente
char id[TAMLEX];		// Utilizado por el analizador léxico
int delantero=-1;		// Utilizado por el analizador léxico
int fin=0;				// Utilizado por el analizador léxico
int numLinea=1;         // Número de LInea
int tabulador=0;        // Eapcio de Tabulación

char *componentes_token[]={
		"STRING",
		"NUMBER",
        "COMA",
		"DOS_PUNTOS",
		"L_LLAVE",
		"R_LLAVE",
		"L_CORCHETE",
		"R_CORCHETE",
		"PR_TRUE",
		"PR_FALSE",
		"PR_NULL"
};

//Función main
int main(int argc,char* args[]){
    initTabla();
	initTablaSimbolos();
    if(argc > 1){
        output = fopen("output.txt", "w");//Archivo output para salida de información
        if (!(fuente=fopen(args[1],"rt"))){
			printf("Archivo fuente no encontrado.\n");
			exit(1);
		}

        while (t.compLex!=EOF || error == 1){
            sigLex();
            switch(tabulador){
                case 1:
                    printf("\t");
                    fprintf(output,"\t");
                    tabulador=0;
                    break;
                case 2:
                    printf(" ");
                    fprintf(output," ");
                    tabulador=0;
                    break;
                case 3:
                    printf("\n");
                    fprintf(output,"\n");
                    tabulador=0;
                    break;
                default:
                    if(t.compLex!=-1){
					    printf("%s",componentes_token[t.compLex-256]);
					    printf(" ");

					    fprintf(output,"%s",componentes_token[t.compLex-256]);
					    fprintf(output," ");
				    }
            }
        }
        printf("\n");
		fclose(fuente);
		fclose(output);
    }else{
        printf("Debe pasar como parametro el path al archivo fuente.\n");
		exit(1);
    }
    return 0;
}

void mensajError(const char* mensaje)
{
	printf("Linea %d: Error Léxico Encontrado. %s.\n",numLinea, mensaje);
	fprintf(output,"Linea %d: Error Lexico Encontrado. %s.\n",numLinea, mensaje);
	error = 1;
	char a;

	while(a!='\n' && a!=EOF){
		a=fgetc(fuente);
	}
}

void sigLex(){
    char a=0;
    int i=0;
    entrada e;
    int acepto=0;
	int estado=0;
    char msg[41];
    while((a=fgetc(fuente))!=EOF){
        if (a=='\t'){//Tabulador
			tabulador = 1;
			break;
		}else if(a==' '){//Espacio en blanco
			tabulador = 2;
			break;
		}else if(a=='\n'){//Salto de Linea
			//Incremento en número de línea
			tabulador = 3;
			numLinea++;
			break;
		}else if(isalpha(a)){ //Palabra reservada ([A-Za-z])
            i=0;//Variable para recorrer el archivo fuente
            do{
				id[i]=tolower(a);//Se acumula en un vector temporal, con caracteres en minúsculas
				i++;//Se recorre el vector
				a=fgetc(fuente);//Se recorre el archivo fuente
				if (i>=TAMLEX) //Si el indice del vector excede al tamaño máximo
					mensajError("Indice fuera de rango, el identificador no puede superar los 50 caracteres.");
			}while(isalpha(a));
            id[i]='\0';//Null para el tipo de datos char
            if (a!=EOF)
                ungetc(a,fuente);
            else
                a=0;
            t.pe=buscar(id);
			t.compLex=t.pe->compLex;
            if (t.pe->compLex==-1){
				mensajError("No es una palabra reservada.");
			}
			break;
        }else if (a=='"'){//LITERAL CADENA '"'=.*
            i=0;
			do{
				id[i]=a;
				i++;
				a=fgetc(fuente);
				if (i>=TAMLEX)
					mensajError("Indice fuera de rango, el identificador no puede superar los 50 caracteres.");
			}while(a!='"' && a!=EOF);//while(isdigit(a) || isalpha(a));
			
			if(a!=EOF){
				id[i]=a;
				id[i+1]='\0';
			}
			
			if(a!='"'){
				mensajError("EOF no válido");
				break;
			}else
                a=0;
			t.pe=buscar(id);
			t.compLex=t.pe->compLex;
			if (t.pe->compLex==-1){
				strcpy(e.lexema,id);
				e.compLex=LITERAL_CADENA;
				insertar(e);
				t.pe=buscar(id);
				t.compLex=LITERAL_CADENA;
			}
			break;
        }else if (isdigit(a)){
		    //es un numero
			i=0;
			estado=0;
			acepto=0;
			id[i]=a;
			while(!acepto){
				switch(estado){
				case 0: //una secuencia netamente de digitos, puede ocurrir . o e
					a=fgetc(fuente);
					if (isdigit(a)){
						id[++i]=a;
						estado=0;
					}else if(a=='.'){
						id[++i]=a;
						estado=1;
					}else if(tolower(a)=='e'){
						id[++i]=a;
						estado=3;
					}else{
						estado=6;
					}
					break;
				case 1://un punto, debe seguir un digito
					a=fgetc(fuente);						
					if (isdigit(a)){
						id[++i]=a;
						estado=2;
					}else{
						sprintf(msg,"No se esperaba '%c'",a);
						estado=-1;
					}
					break;
				case 2://la fraccion decimal, pueden seguir los digitos o e
					a=fgetc(fuente);
					if (isdigit(a)){
						id[++i]=a;
						estado=2;
					}
					else if(tolower(a)=='e'){
						id[++i]=a;
						estado=3;
					}else
						estado=6;
					break;
				case 3://una e, puede seguir +, - o una secuencia de digitos
					a=fgetc(fuente);
					if (a=='+' || a=='-'){
						id[++i]=a;
						estado=4;
					}else if(isdigit(a)){
						id[++i]=a;
						estado=5;
					}else{
						sprintf(msg,"No se esperaba '%c'",a);
						estado=-1;
					}
					break;
				case 4://necesariamente debe venir por lo menos un digito
					a=fgetc(fuente);
					if (isdigit(a)){
						id[++i]=a;
						estado=5;
					}else{
						sprintf(msg,"No se esperaba '%c'",a);
						estado=-1;
					}
					break;
				case 5://una secuencia de digitos correspondiente al exponente
					a=fgetc(fuente);
					if (isdigit(a)){
						id[++i]=a;
						estado=5;
					}else{
						estado=6;
					}
                    break;
				case 6://estado de aceptacion, devolver el caracter correspondiente a otro componente lexico
					if (a!=EOF)
						ungetc(a,fuente);
					else
						a=0;
					id[++i]='\0';
					acepto=1;
					t.pe=buscar(id);
					if (t.pe->compLex==-1){
						strcpy(e.lexema,id);
						e.compLex=LITERAL_NUM;
						insertar(e);
						t.pe=buscar(id);
					}
					t.compLex=LITERAL_NUM;
					break;
				case -1:
					if (a==EOF)
						mensajError("No se esperaba el fin de archivo.");
					else
						mensajError(msg);
					exit(1);
				}
			}
			break;
		}
        else if (a=='['){
			t.compLex=L_CORCHETE;
			t.pe=buscar("[");
			break;
		}else if (a==']'){
			t.compLex=R_CORCHETE;
			t.pe=buscar("]");
			break;
		}else if (a=='{'){
			t.compLex=L_LLAVE;
			t.pe=buscar("{");
			break;
		}else if (a=='}'){
			t.compLex=R_LLAVE;
			t.pe=buscar("}");
			break;
		}else if (a==','){
			t.compLex=COMA;
			t.pe=buscar(",");
			break;
		}else if (a==':'){
			t.compLex=DOS_PUNTOS;
			t.pe=buscar(":");
			break;
		}
	}

	if (a==EOF){
		t.compLex=EOF;
		error = 0;
		sprintf(e.lexema,"EOF");
		t.pe=&e;
	}
}

