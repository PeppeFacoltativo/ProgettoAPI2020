#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INPUT_SIZE 50
#define BUFFER_SIZE 1025

typedef enum { false, true } bool;

/*STRUCTS*/
struct line {
	char *text;
	struct line *nextLine;
	struct line *previousLine;
};

struct file {
	int fileLength;
	struct line *tail;
	struct line *head;
};

struct list {
	int listLength;
	struct node *head;
};

struct node {//validi sia per undoList sia per redoList
	char cmd;//c=aggiunta, x=modifica, d=rimozione, n=nulla
	struct line *firstLine;
	struct line *lastLine;
	struct node *previousNode;
	int newFileLength; //la lunghezza del file prima che venisse eseguita l'istruzione
};
struct list undoList;
struct list redoList;



/*FUNCTIONS*/

char *CopyString(char *destination, char *source)
{
	char *j = source;
	int sourceLength = strlen(source);
	destination = calloc(sourceLength + 2, sizeof(char));
	for (char *i = destination; i < destination + sourceLength; i++)
	{
		*i = *j;
		j++;
		//printf("%c", destination[i]);
	}
	destination[sourceLength + 1] = '\0';
	//printf("%s", destination);

	return destination;
}


void FindLines(struct file *f, int pos1, int pos2, struct line **temp1, struct line **temp2) //pos è il numero di riga
{
	
	if (pos1 <= f->fileLength / 2) //linetochange
	{
		(*temp1) = f->head;
		for (int i = pos1 - 1; i--;)
			(*temp1) = (*temp1)->nextLine;
	}
	else
	{
		(*temp1) = f->tail;
		for (int i = f->fileLength - pos1; i--;)
			(*temp1) = (*temp1)->previousLine;
	}

	if (pos1 != pos2)
	{
		if (pos2 < pos1 / 2)
		{
			(*temp2) = f->head;
			for (int i = pos2 - 1; i--;)
				(*temp2) = (*temp2)->nextLine;
		}
		else if (pos2 >= pos1 / 2 && pos2 <= pos1)
		{
			(*temp2) = (*temp1);
			for (int i = pos1 - pos2; i--;)
				(*temp2) = (*temp2)->previousLine;
		}
		else if (pos2 >= pos1 && pos2 <=(pos1 + f->fileLength) / 2)
		{
			(*temp2) = (*temp1);
			for (int i = pos2 - pos1; i--;)
				(*temp2) = (*temp2)->nextLine;
		}
		else
		{
			(*temp2) = f->tail;
			for (int i = f->fileLength - pos2; i--;)
				(*temp2) = (*temp2)->previousLine;
		}
	}
	else
		*temp2 = *temp1;
}

struct line *FindLine(struct file *f, int pos) //pos è il numero di riga
{

	struct line *fetchedLine;
	if (pos <= f->fileLength / 2) //linetochange
	{
		fetchedLine = f->head;
		for (int i = pos - 1; i--;)
			fetchedLine = fetchedLine->nextLine;
	}
	else
	{
		fetchedLine = f->tail;
		for (int i = f->fileLength - pos; i--;)
			fetchedLine = fetchedLine->previousLine;
	}

	return fetchedLine;
}

void PushNode(struct list *l, char command, struct line *fl, struct line *ll, int fileLength)//al posto di l dovremo passare undolist o redolist
{
	struct node *temp;
	temp = malloc(sizeof(struct node));
	temp->cmd = command;
	temp->newFileLength = fileLength;

	//printf("\n%s",fl->text);
	temp->firstLine = fl;
	//printf("\n%s",ll->text);
	temp->lastLine = ll;

	if (l->head != NULL) //Lista non vuota
	{
		temp->previousNode = l->head;
		l->head = temp;
	}
	else //Lista vuota
	{
		temp->previousNode = NULL;
		l->head = temp;
	}
	l->listLength = l->listLength + 1;
}



void Undo(struct file *f, int loops)                          //++++++++++++++++++DA TESTARE++++++++++++++++
{
	for(int k=loops; k--; )
	{
		if (undoList.head != NULL)
		{
			switch (undoList.head->cmd)
			{
			case 'c': //aggiunta
				if (undoList.head->firstLine->previousLine != NULL) //La prima linea da reinserire non è la head
					undoList.head->firstLine->previousLine->nextLine = NULL;
				else
				{
					//if(undoList.head->lastLine == NULL) //Avevo aggiunto solo una linea
					if (undoList.head->lastLine->nextLine != NULL)
						f->head = undoList.head->lastLine->nextLine;
					else
					{
						f->head = NULL;
						f->tail = NULL;
					}
				}

				PushNode(&redoList, 'c', undoList.head->firstLine, undoList.head->lastLine, f->fileLength);

				f->fileLength = undoList.head->newFileLength;
				break;
			case 'x':; //sostituzione

				struct line *h = NULL, *t = NULL;

				if (undoList.head->firstLine->previousLine != NULL)
				{
					h = undoList.head->firstLine->previousLine->nextLine;
					undoList.head->firstLine->previousLine->nextLine = undoList.head->firstLine;
				}
				else
				{
					h = f->head;
					f->head = undoList.head->firstLine;
				}

				if (undoList.head->lastLine->nextLine != NULL)
				{
					t = undoList.head->lastLine->nextLine->previousLine;
					undoList.head->lastLine->nextLine->previousLine = undoList.head->lastLine;
				}
				else
				{
					t = f->tail;
					f->tail = undoList.head->lastLine;
				}

				PushNode(&redoList, 'x', h, t, f->fileLength);

				f->fileLength = undoList.head->newFileLength;

				break;
			case 'd': //cancellazione

				PushNode(&redoList, 'd', undoList.head->firstLine, undoList.head->lastLine, f->fileLength);

				if (undoList.head->firstLine->previousLine != NULL) //Non è la head del file
					undoList.head->firstLine->previousLine->nextLine = undoList.head->firstLine;
				else
					f->head = undoList.head->firstLine;

				if (undoList.head->lastLine->nextLine != NULL) //non è la tail del file
					undoList.head->lastLine->nextLine->previousLine = undoList.head->lastLine;
				else
					f->tail = undoList.head->lastLine;

				f->fileLength = undoList.head->newFileLength;

				break;
			case 'n':
				PushNode(&redoList, 'n', undoList.head->firstLine, undoList.head->lastLine, f->fileLength);
				break;
			default:
				break;
			}
			/*
			if (f->head == NULL && f->tail == NULL)
				printf("a");
				*/

			struct node *temp = undoList.head;
			if (undoList.head->previousNode != NULL)
			{
				undoList.head = undoList.head->previousNode;
				undoList.listLength = undoList.listLength - 1;
			}
			else
			{
				undoList.head = NULL;
				undoList.listLength = 0;
			}
			free(temp);
		}
	}
}

void Redo(struct file *f, int loops)                          //++++++++++++++++++DA TESTARE++++++++++++++++
{
	for (int k=loops; k--; )
	{
		if (redoList.head != NULL)
		{
			switch (redoList.head->cmd)
			{
			case 'c': //aggiunta
				if (redoList.head->firstLine->previousLine != NULL) //La prima linea da reinserire non è la head
					redoList.head->firstLine->previousLine->nextLine = redoList.head->firstLine;
				else
				{
					f->head = redoList.head->firstLine;
					f->tail = redoList.head->lastLine;
				}

				PushNode(&undoList, 'c', redoList.head->firstLine, redoList.head->lastLine, f->fileLength);

				f->fileLength = redoList.head->newFileLength;
				break;
			case 'x':; //sostituzione

				struct line *h = NULL, *t = NULL;

				if (redoList.head->firstLine->previousLine != NULL)
				{
					h = redoList.head->firstLine->previousLine->nextLine;
					redoList.head->firstLine->previousLine->nextLine = redoList.head->firstLine;
				}
				else
				{
					h = f->head;
					f->head = redoList.head->firstLine;
				}

				if (redoList.head->lastLine->nextLine != NULL)
				{
					t = redoList.head->lastLine->nextLine->previousLine;
					redoList.head->lastLine->nextLine->previousLine = redoList.head->lastLine;
				}
				else
				{
					t = f->tail;
					f->tail = redoList.head->lastLine;
				}

				PushNode(&undoList, 'x', h, t, f->fileLength);

				f->fileLength = redoList.head->newFileLength;


				break;
			case 'd': //cancellazione

				if (redoList.head->firstLine->previousLine != NULL) //Non devo ricancellare la head
				{
					if (redoList.head->lastLine->nextLine != NULL)
					{
						redoList.head->firstLine->previousLine->nextLine = redoList.head->lastLine->nextLine;
						redoList.head->lastLine->nextLine->previousLine = redoList.head->firstLine->previousLine;
					}
					else
					{
						redoList.head->lastLine->nextLine = NULL;
						f->tail = redoList.head->lastLine;
					}
				}
				else
				{
					if (redoList.head->lastLine->nextLine != NULL)
					{
						f->head = redoList.head->lastLine->nextLine;
						redoList.head->lastLine->nextLine->previousLine = NULL;
					}
					else
					{
						f->head = NULL;
						f->tail = NULL;
					}
				}

				PushNode(&undoList, 'd', redoList.head->firstLine, redoList.head->lastLine, f->fileLength);

				f->fileLength = redoList.head->newFileLength;

				break;
			case 'n':
				PushNode(&undoList, 'n', redoList.head->firstLine, redoList.head->lastLine, f->fileLength);
				break;
			default:
				break;
			}

			struct node *temp = redoList.head;
			if (redoList.head->previousNode != NULL)
			{
				redoList.head = redoList.head->previousNode;
				redoList.listLength = redoList.listLength - 1;
			}
			else
			{
				redoList.head = NULL;
				redoList.listLength = 0;
			}
			free(temp);
		}
	}
}


void DeleteLines(struct file *f, int ind1, int ind2)
{
	struct line *tempH = NULL, *tempT = NULL;
	FindLines(f, ind1, ind2, &tempH, &tempT);

	/*
	tempH = FindLine(f, ind1);
	if (ind1 != ind2)
		tempT = FindLine(f, ind2);
	else
		tempT = tempH;
	*/

	if (tempH->previousLine != NULL) //TempH non è la head
		tempH->previousLine->nextLine = tempT->nextLine;
	else
	{
		if (tempT->nextLine != NULL)
			f->head = tempT->nextLine;
		else
		{
			f->head = NULL;
			f->tail = NULL;
		}
	}

	if (tempT->nextLine != NULL) //TempT non è la tail
		tempT->nextLine->previousLine = tempH->previousLine;
	else
	{
		if (tempH->previousLine != NULL)
			f->tail = tempT->previousLine;
		else
			f->tail = NULL;
	}
		
	//AGGIUNGI LA LINEA RIMOSSA ALL'UNDOLIST
	PushNode(&undoList, 'd', tempH, tempT, f->fileLength);

	f->fileLength = f->fileLength - ind2 + ind1 - 1;
}

void ReplaceLines(struct file *f, struct line *newFirstLine, struct line *oldFirstLine, struct line *newLastLine, struct line *oldLastLine, int newFileLength) //newFileLength la nuova lunghezza del file
{
	//AGGIUNGI LA LINEA SOSTITUITA ALL'UNDOLIST
	PushNode(&undoList, 'x', oldFirstLine, oldLastLine, f->fileLength);

	newFirstLine->previousLine = oldFirstLine->previousLine;
	newLastLine->nextLine = oldLastLine->nextLine;

	if (newFirstLine->previousLine != NULL) //newFirstLine Non è la head
	{
		newFirstLine->previousLine->nextLine = newFirstLine;
		//printf("%s\n", newLine->previousLine->text);
	}
	else
		f->head = newFirstLine;

	if (newLastLine->nextLine != NULL) //newLastLine non è la tail
	{
		newLastLine->nextLine->previousLine = newLastLine;
		//printf("%s\n", newLine->nextLine->text);
	}
	else
		f->tail = newLastLine;

	if (f->fileLength != newFileLength)
		f->fileLength = newFileLength;

}

struct line *EnqueueLine(struct file *f, char *data)
{
	struct line *temp;
	temp = malloc(sizeof(struct line));
	temp->text = CopyString(NULL, data);
	//printf("%s\n", temp->text);
	temp->nextLine = NULL;
	if (f->fileLength > 0)//File non vuoto
	{
		temp->previousLine = f->tail;
		f->tail->nextLine = temp;
		f->tail = temp;
	}
	else
	{
		temp->previousLine = NULL;
		f->tail = temp;
		f->head = temp;
	}
	f->fileLength++;

	//linesOrder + f->fileLength = malloc(sizeof(struct line*));
	return temp;
}

void Print(struct file *f, int ind1, int ind2)
{
	struct line *l = f->head;
	//printf("%s\n", l->text);
	//printf("%s\n", l->nextLine->text);
	if (ind2 < 1)
		fputs(".\n", stdout);
	else
	{
		for (int i = 1; i <= ind2; ++i)
		{
			if (i >= ind1)
			{
				if (i <= f->fileLength)
					fputs(l->text, stdout);
				else
					fputs(".\n", stdout);
			}
			if (l != NULL && l->nextLine != NULL)
				l = l->nextLine;
		}
	}
	//printf("File Length:%d\n", f.fileLength);
}

/*FUNCTIONS*/



int main() {

	//FILE OPEN
	FILE *fptr = stdin;
	//fptr=fopen("C:\Rolling_Back_1_input.txt", "r");

	struct file f;
	f.fileLength = 0;
	f.head = NULL;
	f.tail = NULL;

	undoList.listLength = 0;
	redoList.listLength = 0;

	char input[INPUT_SIZE];
	//bool lastcmdUndo = false;
	//int previousUndoValue = 0;

	while (fgets(input, INPUT_SIZE, fptr) != NULL)
	{
		/*if (f.fileLength > 0 && f.head == NULL)
			printf("a");

		strcpy(a,input);
		if (f.head == NULL && f.tail != NULL)
			printf("a");*/
		//printf("%s\n", input);
		int lastCharPos = strlen(input) - 2; //Lunghezza della stringa in input
		//printf("%ld\n", strlen(input));
		char cmd = input[lastCharPos]; //la lettera che indica il comando da eseguire
		//printf("%c\n", cmd);
		input[lastCharPos] = input[lastCharPos + 1];

		if (cmd == 'u' || cmd == 'r')//UNDO**************************************************************************************************************************
		{
			char *ptr;
			ptr = strtok(input, "^,");
			int countUndo = atoi(ptr);

			if (cmd == 'u')
			{
				if (countUndo > undoList.listLength)
					countUndo = undoList.listLength;
			}
			else
			{
				if (countUndo > redoList.listLength)
					countUndo = -redoList.listLength;
				else
					countUndo = -countUndo;
			}

			while (1)
			{
				fgets(input, INPUT_SIZE, fptr);
				lastCharPos = strlen(input) - 2; //Lunghezza della stringa in input
				cmd = input[lastCharPos]; //la lettera che indica il comando da eseguire
				input[lastCharPos] = input[lastCharPos + 1];
				if (cmd == 'r')
				{
					char *ptr;
					ptr = strtok(input, "^,");
					int countR = atoi(ptr);
					countUndo -= countR;	
					if (countUndo < -redoList.listLength)
						countUndo = -redoList.listLength;
				}
				else if (cmd == 'u')
				{
					char *ptr;
					ptr = strtok(input, "^,");
					int countU = atoi(ptr);
					countUndo += countU;
					if (countUndo > undoList.listLength)
						countUndo = undoList.listLength;
				}
				else
					break;
			};

			if (countUndo > 0)
				Undo(&f, countUndo);	
			else
				Redo(&f, -countUndo);
		}
		/*if (cmd == 'r')//REDO*********************************************************************************************************************
		{
			char *ptr;
			ptr = strtok(input, "^,");
			int count = atoi(ptr);
			Redo(&f, count);
		}
		break;*/
		if (cmd == 'q') //QUIT********************************************************************************************************************
		{
			fclose(fptr);
			exit(0);
		}
		if (cmd == 'c') //WRITE*******************************************************************************************************************
		{
			//lastcmdUndo = false;

			char buffer[BUFFER_SIZE];
			char *ptr;
			ptr = strtok(input, "^,");
			//printf("%s\n", ptr);
			int ind1 = atoi(ptr);			//ind1 è il primo numero
			//printf("%d\n", ind1);
			ptr = strtok(NULL, "^,");
			int ind2 = atoi(ptr);			//ind2 è il secondo numero
			//printf("%d\n", ind2);
			int numOfLines = ind2 - ind1 + 1;

			if (ind1 > f.fileLength) //Aggiunta
			{
				struct line *first = NULL;
				struct line *last = NULL;
				for (int i = 0; i < numOfLines + 1; ++i)
				{
					fgets(buffer, 1024, fptr);
					if (i < numOfLines && numOfLines > 1)
					{
						if (i == 0)//prima linea inserita
							first = EnqueueLine(&f, buffer);
						else if (i == numOfLines - 1)//prima linea inserita
							last = EnqueueLine(&f, buffer);
						else
							EnqueueLine(&f, buffer);
					}
					if (i < numOfLines && numOfLines == 1)
						first = last = EnqueueLine(&f, buffer);
				}

				//bisogna salvare nell'undolist
				PushNode(&undoList, 'c', first, last, f.fileLength - numOfLines);

			}
			else
			{
				struct file *tempNewLines = malloc(sizeof(struct file));//Lista temporanea contentente le linee che sostituiranno quelle vecchie
				struct line *firstLineToChange;//La "head" delle linee da sostituire
				struct line *lastLineToChange;//La "tail" delle linee da sostituire

				struct line *prevLine = NULL;//la linea inserita precedentemente nel ciclo
				for (int i = ind1; i <= ind2 + 1; ++i)
				{
					fgets(buffer, 1024, fptr);
					if (i <= ind2)
					{
						struct line *temp;
						temp = malloc(sizeof(struct line));
						temp->text = CopyString(NULL, buffer);
						temp->nextLine = NULL;
						if (prevLine != NULL)
						{
							temp->previousLine = prevLine;
							prevLine->nextLine = temp;
						}
						prevLine = temp;

						if (i == ind1)//La head dei nuovi inserimenti
							tempNewLines->head = temp;
						if (i == ind2)//La tail dei nuovi inserimenti
							tempNewLines->tail = temp;
					}
				}

				if (ind2 < f.fileLength) //modifica
				{
					/*
					firstLineToChange = FindLine(&f, ind1);
					lastLineToChange = FindLine(&f, ind2);
					*/
					FindLines(&f, ind1, ind2, &firstLineToChange, &lastLineToChange);
					ReplaceLines(&f, tempNewLines->head, firstLineToChange, tempNewLines->tail, lastLineToChange, f.fileLength);
				}
				else //modifica e aggiunta
				{
					firstLineToChange = FindLine(&f, ind1);
					lastLineToChange = f.tail;
					ReplaceLines(&f, tempNewLines->head, firstLineToChange, tempNewLines->tail, lastLineToChange, ind2);
				}

			}
			
			if (redoList.head != NULL) //Libera celle redolist
			{
				struct node *temp = redoList.head;
				while (temp != NULL)
				{
					temp = redoList.head->previousNode;
					free(redoList.head);
					redoList.head = temp;
				}
				redoList.listLength = 0;
			}
		}
		if (cmd == 'd')//DELETE*****************************************************************************************************************
		{

			//lastcmdUndo = false;

			char *ptr;
			ptr = strtok(input, "^,");
			//printf("%s\n", ptr);
			int ind1 = atoi(ptr);			//ind1 è il primo numero
			//printf("%d\n", ind1);
			ptr = strtok(NULL, "^,");
			int ind2 = atoi(ptr);			//ind2 è il secondo numero
			//printf("%d\n", ind2);

			if (ind1 <= f.fileLength)
			{
				if (ind2 > f.fileLength)
					ind2 = f.fileLength;

				DeleteLines(&f, ind1, ind2);
			}
			else
				PushNode(&undoList, 'n', NULL, NULL, f.fileLength);

			if (redoList.head != NULL) //Libera celle redolist
			{
				struct node *temp = redoList.head;
				while (temp != NULL)
				{
					temp = redoList.head->previousNode;
					free(redoList.head);
					redoList.head = temp;
				}
				redoList.listLength = 0;
			}

		}
		if (cmd == 'p')//PRINT****************************************************************************************************************
		{
			//lastcmdUndo = false;

			char *ptr;
			ptr = strtok(input, "^,");
			//printf("%s\n", ptr);
			int ind1 = atoi(ptr);			//ind1 è il primo numero
			//printf("%d\n", ind1);
			ptr = strtok(NULL, "^,");
			int ind2 = atoi(ptr);			//ind2 è il secondo numero
			//printf("%d\n", ind2);

			Print(&f, ind1, ind2);

		}
		/*
		else if (cmd == 'x')
		printf("a");*/
		
	}
	fclose(fptr);

	return 0;
}