//LIBRARYS
#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <matrix.h>
#include <time.h>
// ---------------------------------------------------------------------------
// MARTIAN
// ---------------------------------------------------------------------------

typedef struct node{
  ALLEGRO_MUTEX *mutex;
  ALLEGRO_COND *cond;
  int column;
  int row;
  int energy;
  int static_energy;
  int period;
  int period_counter;
  int id;
  int goTo;
  bool isReady_New;
  bool isDone;
  bool isActive;
  bool inProgress;
  ALLEGRO_THREAD *thread_id;
  struct node *next;
}
node;

struct report{
  int data;
  struct report *next;
};

struct report *head_report = NULL;

#define MARTIAN_DEFAULT (     \
    (node){                   \
        .mutex = NULL,        \
        .cond = NULL,         \
        .column = 0,          \
        .row = 224,           \
        .period = 0,          \
        .period_counter = 0,  \
        .energy = 0,          \
        .static_energy = 0,   \
        .id = 0,              \
        .goTo = 2,            \
        .isReady_New = false, \
        .isDone = false,      \
        .isActive = false,    \
        .inProgress = false,  \
        .next = NULL,         \
        .thread_id = NULL,    \
    })

// ---------------------------------------------------------------------------
// MAIN 
// ---------------------------------------------------------------------------

const float FPS = 30;
const int SCREEN_W = 608;
const int SCREEN_H = 608;
const int BOUNCER_SIZE = 32;
int new_martian = 0;
int energy = 0;
int period = 0;
int __algorithm;
int __mode;
int __start_auto = 0; // 0 : false
int __ok = 0;         // 0 : false

float U = 0;       // 0 : false
float __RM_Ci = 0; // 0 : false
float __RM_Pi = 0; // 0 : false

int all_news = 0;    // All Thread (Martians) are ready
int is_all_news = 0; // All Thread (Martians) are ready
char energyLine[10] = "";

ALLEGRO_BITMAP *martian_img;

ALLEGRO_DISPLAY *display;
ALLEGRO_FONT *font;
#define BLACK (al_map_rgb(0, 0, 0))
#define POINT 0, 0, 0
static void *Func_Thread(ALLEGRO_THREAD *thr, void *arg);

// ---------------------------------------------------------------------------
// EVENT HANDLER 
// ---------------------------------------------------------------------------

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define CODE_CONTINUE (MIN(EXIT_SUCCESS, EXIT_FAILURE) - 1)
#define CODE_SUCCESS EXIT_SUCCESS
#define CODE_FAILURE EXIT_FAILURE
typedef int code;
code HandleEvent(ALLEGRO_EVENT ev);
void endGame();
void freeMartians();
void clearListMartians();
float formuleU();

// ---------------------------------------------------------------------------
// REDRAW HANDLER 
// ---------------------------------------------------------------------------

int length();
bool REDRAW_IS_READY;
void RedrawSetReady(void);
void RedrawClearReady(void);
bool RedrawIsReady(void);
void RedrawDo(); // to be defined in Implementation of EVENT HANDLER Module

void printReportConsole(){

  struct report *ptrReport = head_report;
  printf("\nMAZE GAME REPORT WITH ALGORITHMS EDF OR RM \n< ");
  while (ptrReport != NULL){
    printf("|%d", ptrReport->data);
    ptrReport = ptrReport->next;
  }
  printf("| >\n");
}

void addLast(struct report **head, int val){
  //create a new node
  struct report *newNode = malloc(sizeof(struct report));
  newNode->data = val;
  newNode->next = NULL;

  //If head is NULL, it is an empty list
  if (*head == NULL)
    *head = newNode;
  //Otherwise, find the last report and add the newNode
  else{
    struct report *lastNode = *head;

    //last report's next address will be NULL.
    while (lastNode->next != NULL){
      lastNode = lastNode->next;
    }

    //add the newNode at the end of the linked list
    lastNode->next = newNode;
  }
}

// ---------------------------------------------------------------------------
// MARTIANS ACTIONS
// ---------------------------------------------------------------------------

struct node *head = NULL;
struct node *current = NULL;
// ---------------------------------------------------------------------------

// display the list of martian - console
void printListMartians(){
  node *ptr = NULL;
  ptr = head;
  printf("\n[ ");
  while (ptr != NULL){ //start from the beginning 
    printf("(%d,%d) ", ptr->energy, ptr->period);
    ptr = ptr->next;}
  printf(" ]\n");
  }

void freeMartians(){
  node *ptr = NULL;
  ptr = head;
  while (ptr != NULL){ //start from the beginning
    al_destroy_thread(ptr->thread_id);
    ptr = ptr->next;
  }
}

//render list
void renderListMartians(){
  node *martianTemp = head;   // Header List
  while (martianTemp != NULL){ //start from the beginning
    RedrawDo(martianTemp, martian_img); // Pain image martian on screen
    martianTemp = martianTemp->next;    // Follow martian
  }
}

void stopMartians(){
  node *martianTemp = head;   // Header List
  while (martianTemp != NULL){ //start from the beginning
    martianTemp->isActive = false;   // Follow martian
    martianTemp = martianTemp->next; // Follow martian
  }
}

void clearListMartians(){
  head = NULL;
}

//insert link at the first location
void addMartian(int energy, int period){
  
  __RM_Ci = __RM_Ci + energy;
  __RM_Pi = __RM_Pi + period;

  U = __RM_Ci / __RM_Pi;
  node *link = (node *)malloc(sizeof(node)); //create a link

  assert(link);
  *link = MARTIAN_DEFAULT;//Set default values
  link->mutex = al_create_mutex();// Mutex
  link->cond = al_create_cond();// Condition
  assert(link->mutex);
  assert(link->cond);
  link->energy = energy;
  link->static_energy = energy;
  link->period = period;
  link->period_counter = period;
  link->id = length() + 1;
  link->isReady_New = false;
  link->inProgress = false;
  link->isDone = false;
  link->next = head; //point it to old first node
  head = link;//point first to new first node
  ALLEGRO_THREAD *martianThread_ = al_create_thread(Func_Thread, link);
  link->thread_id = martianThread_;//Add ID Thread to martian struct
  printf("\n¡NEW ALIEN!\n");
  al_start_thread(martianThread_);//Start Martian Thread
}

//is list empty
bool isEmpty(){
  return head == NULL;
}

int length(){
  int length = 0;
  node *current;
  for (current = head; current != NULL; current = current->next)
    length++;
  return length;
}

//findMartianID a link with given energy
struct node *findMartianID(int id){
  node *current = head; //start from the first link
  if (head == NULL)     //if list is empty
    return NULL;

  while (current->id != id){ //navigate through list
    if (current->next == NULL)
      return NULL; //if it is last node
    else
      current = current->next; //go to next link
  }
  return current; //if id found, return the current Link
}

struct node *findEDF_Martian(){
  if (head == NULL) //if list is empty
    return NULL;

  node *current = head;//start from the first link
  int p_counter = 9999;// Defaul Value
  node *martianTemp = head;// Header List
  while (martianTemp != NULL){ //start from the beginning
    printf("%d. E:%d P:%d CounterP:%d \tisDone: %d isReady_New: %d inProgress: %d  isActive: %d\n",
           martianTemp->id,
           martianTemp->energy,
           martianTemp->period,
           martianTemp->period_counter,
           martianTemp->isDone,
           martianTemp->isReady_New,
           martianTemp->inProgress,
           martianTemp->isActive);
    if (martianTemp->period_counter < p_counter && martianTemp->isDone == 0){ // martianTemp->isReady_New = true;
      p_counter = martianTemp->period_counter;
      current = martianTemp;
    }
    martianTemp = martianTemp->next; // Follow martian
  }

  if (current->isDone == true)
    return NULL;  //if energy found, return the current Link
  return current; //if energy found, return the current Link
}

struct node *findRM_Martian(){
  if (head == NULL) //if list is empty
    return NULL;

  node *current = head;//start from the first link
  int energy_1 = 9999;// Defaul Value
  node *martianTemp = head;// Header List
  while (martianTemp != NULL){ //start from the beginning
    if (martianTemp->static_energy <= energy_1 && martianTemp->isDone == false){ // martianTemp->isReady_New = true;
      energy_1 = martianTemp->static_energy;
      current = martianTemp;
    }
    martianTemp = martianTemp->next; // Follow martian
  }

  if (current->isDone == true)
    return NULL;  //if energy found, return the current Link
  return current; //if energy found, return the current Link
}

void resetEnergy(int sGame){
  if (head != NULL){
    node *martianTemp = head;//Header List
    int residuo = 0;
    while (martianTemp != NULL){//start from the beginning
      residuo = sGame % martianTemp->period;

      if (residuo == 0){
        al_lock_mutex(martianTemp->mutex); // Lock Mutex
        martianTemp->isDone = false;
        martianTemp->isReady_New = true;
        martianTemp->inProgress = false;
        martianTemp->energy = martianTemp->static_energy;
        martianTemp->period_counter = martianTemp->period;
        al_unlock_mutex(martianTemp->mutex); // UnLock Mutex
      }
      martianTemp = martianTemp->next; // Follow martian
    }
  }
}

void reduceCounterPeriod(){
  if (head != NULL){
    node *martianTemp = head;// Header List
    while (martianTemp != NULL){//start from the beginning
      al_lock_mutex(martianTemp->mutex); // Lock Mute
      martianTemp->period_counter--;
      if (martianTemp->period_counter == 0)
        martianTemp->period_counter = martianTemp->period;
      al_unlock_mutex(martianTemp->mutex); // UnLock Mutex
      martianTemp = martianTemp->next;     // Follow martian
    }
  }
}

void endGame(){

}

// ---------------------------------------------------------------------------
// Implementation of MAIN Module
// ---------------------------------------------------------------------------
float formuleU(){
  int Ci = 0;
  int Pi = 0;

  node *current;
  for (current = head; current != NULL; current = current->next){
    Ci = Ci + current->energy;
    Pi = Pi + current->period;
  }

  printf("\tCi : %d\n", Ci);
  printf("\tPi : %d\n", Pi);
  float ui = (float)Ci / (float)Pi;
  printf("\ttCi/Pi : %f\n", ui);

  if (Pi == 0)
    return -1;
  else
    return ui;
}

int parameterAlgorithm(int argc, char *argv[]){
  if (argc != 3){
    puts("Atention!, Please enter two input values (Algorithm and Mode)");
    return 0;
  }

  if (!strcmp(argv[1], "RM")){
    __algorithm = 0;
    if (!strcmp(argv[2], "Manual")){
      __mode = 0;
      return 1;
    }
    else if (!strcmp(argv[2], "Automatic")){
      __mode = 1;
      __start_auto = 0;
      return 1;
    }
    else{
      puts("Mode error: <Manual or Automatic>");
      return 0;
    }
  }
  else if (!strcmp(argv[1], "EDF")){
    __algorithm = 1;
    if (!strcmp(argv[2], "Manual")){
      __mode = 0;
      return 1;
    }
    else if (!strcmp(argv[2], "Automatic")){
      __mode = 1;
      __start_auto = 0;
      return 1;
    }
    else{
      puts("Mode error: <Manual or Automatic>");
      return 0;
    }
  }
  else{
    puts("Algorithm error: <RM or EDF>");
    return 0;
  }
}

int main(int argc, char *argv[]){
  if (!parameterAlgorithm(argc, argv))
    return 0;
  al_init();
  if (!al_install_keyboard()){
    puts("couldn't initialize keyboard");
    return 1;
  }
  if (!al_init_image_addon()){
    puts("couldn't initialize image addon");
    return 1;
  }
  font = al_create_builtin_font(); // Font Styles
  if (!font){
    puts("couldn't initialize font");
    return 1;
  }

  ALLEGRO_TIMER *timer = al_create_timer(1.0 / FPS);//Clock : Timer - Allegro display screen
  display = al_create_display(SCREEN_W, SCREEN_H);//Screen
  martian_img = al_load_bitmap("./src/martian.png");//Martian Image
  ALLEGRO_BITMAP *maze_img = al_load_bitmap("./src/maze.jpg");//Maze Image

  al_set_target_bitmap(martian_img);
  al_set_target_bitmap(al_get_backbuffer(display));
  ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
  al_register_event_source(event_queue, al_get_display_event_source(display));
  al_register_event_source(event_queue, al_get_timer_event_source(timer));
  al_register_event_source(event_queue, al_get_keyboard_event_source());
  al_clear_to_color(BLACK);
  al_flip_display();

  // Start timer
  al_start_timer(timer);

  // Event loop
  int code = CODE_CONTINUE;
  char snum[5];
  char senergy[5];
  char speriod[5];
  char smode[10];
  char smmove[10];

  int s30 = 0;
  int sGame = 0;
  int martianID = -1;
  int preview_martianID = -1;

  char ch = '/';
  int current_seconds = 0;
  float u = 0;

  ALLEGRO_THREAD *current_thread = NULL;
  node *martian_found = NULL;

// ---------------------------------------------------------------------------
// ENDVALUES FOR THE MARTIANS WITH EDF AND RM 
// ---------------------------------------------------------------------------

  while (code == CODE_CONTINUE){
    al_draw_bitmap(maze_img, POINT);

    if (RedrawIsReady() && al_is_event_queue_empty(event_queue)){

      if (__mode == 0 || __start_auto == 1){
        s30++;
        if (s30 >= 30){//Un segundo
          s30 = 0;
          sGame++;

          if (strlen(energyLine) > 9)
            strncpy(energyLine, "", 10);

          if (length() != 0){//valida si existe un hilo anterior (para detenerlo)
            stopMartians();

            if (__algorithm == 0)
              martian_found = findRM_Martian();
            else
              martian_found = findEDF_Martian();

            if (martian_found != NULL){
              martianID = martian_found->id;
              addLast(&head_report, martianID);
              al_lock_mutex(martian_found->mutex);//Lock Mutex
              martian_found->inProgress = true;//Martian Thread
              martian_found->isActive = true;//Martian Thread
              martian_found->isDone = false;//Martian Thread
              martian_found->isReady_New = false;//Martian in used
              al_unlock_mutex(martian_found->mutex);//UnLock Mutex

              if (preview_martianID != martianID){
                preview_martianID = martianID;
                strncpy(energyLine, "", 10);
                strncat(energyLine, &ch, 1);
              }
              else{
                strncat(energyLine, &ch, 1);
              }
              al_lock_mutex(martian_found->mutex);// Lock Mutex
              martian_found->energy--;//Martian Movement
              al_unlock_mutex(martian_found->mutex);//UnLock Mutex
              reduceCounterPeriod();
              al_lock_mutex(martian_found->mutex);//Lock Mutex

              if (martian_found->energy == 0){
                martian_found->isDone = true;//Martian Thread
                martian_found->inProgress = false;//Martian Thread
              }
              else{
                martian_found->inProgress = true;//Martian Thread
              }
              al_unlock_mutex(martian_found->mutex);//UnLock Mutex
            }
            else{
              addLast(&head_report, 0);
              current_seconds = 0;
              martianID = 0;
            }
            resetEnergy(sGame);
          }
        }
        renderListMartians();
        if (__algorithm == 0 && U > 0 && U <= 0.69314718056)
          al_draw_text(font, al_map_rgb(0, 0, 0), 580, 590, 0, "UCR");
        else if (__algorithm == 1 && U > 0 && U <= 1)
          al_draw_text(font, al_map_rgb(0, 0, 0), 580, 590, 0, "UCR");
        else
          al_draw_text(font, al_map_rgb(0, 0, 0), 580, 590, 0, "UCR");
      }

      if (martianID > 0){
        sprintf(smode, "%d", martianID);
        al_draw_text(font, al_map_rgb(0, 0, 0), 440, 10, 0, smode);
        al_draw_text(font, al_map_rgb(0, 0, 0), 520, 10, 0, energyLine);
      }

      al_draw_text(font, al_map_rgb(0, 0, 0), 10, 590, 0, "MODE: ");
      if (__mode == 0)
        al_draw_text(font, al_map_rgb(0, 0, 0), 55, 590, 0, "MANUAL");
      else
        al_draw_text(font, al_map_rgb(0, 0, 0), 55, 590, 0, "AUTOMATIC");

      if (__algorithm == 0)
        al_draw_text(font, al_map_rgb(0, 0, 0), 285, 590, 0, "RM");
      else
        al_draw_text(font, al_map_rgb(0, 0, 0), 285, 590, 0, "EDF");

      if (new_martian){
        sprintf(senergy, "%d", energy);
        al_draw_text(font, al_map_rgb(0, 0, 0), 10, 10, 0, "ENERGY: ");
        al_draw_text(font, al_map_rgb(0, 0, 0), 70, 10, 0, senergy);

        sprintf(speriod, "%d", period);
        al_draw_text(font, al_map_rgb(0, 0, 0), 108, 10, 0, "PERIOD: ");
        al_draw_text(font, al_map_rgb(0, 0, 0), 170, 10, 0, speriod);
      }
      al_draw_text(font, al_map_rgb(0, 0, 0), 460, 10, 0, "ENERGY: ");

      al_flip_display();
    }
    ALLEGRO_EVENT ev;
    al_wait_for_event(event_queue, &ev);
    code = HandleEvent(ev);
  }

// ---------------------------------------------------------------------------
// PRINT REPORT 
// ---------------------------------------------------------------------------

  al_clear_to_color(al_map_rgb(0, 0, 0));

  int step_Vertical = 50;
  int step_Horizontal = 30;
  int new_step_Vertical = 0;

  char reportID[5];
  int i;
  al_draw_text(font, al_map_rgb(255, 255, 255), 241, 20, 0, "MAZE GAME REPORT OF THE ALGORITHMS RM OR EDF");
  if (__algorithm == 0)
    al_draw_text(font, al_map_rgb(70, 70, 70), 293, 35, 0, "RM");
  else
    al_draw_text(font, al_map_rgb(70, 70, 70), 290, 35, 0, "EDF");

  /*Vertical*/
  for (i = 0; i < length(); ++i){
    sprintf(reportID, "%d", i + 1);
    al_draw_text(font, al_map_rgb(178, 178, 178), 10, step_Vertical, 0, reportID);
    step_Vertical = step_Vertical + 20;
  }
  step_Vertical = 30;

  /* Columns Data */
  struct report *ptrReportColumns = head_report;
  int iterations = 0; // 20

  while (ptrReportColumns != NULL){
    if (iterations > 20){
      break;
    }
    iterations++;
    new_step_Vertical = step_Vertical + (20 * ptrReportColumns->data);
    if (new_step_Vertical != 30){
      al_draw_text(font, al_map_rgb(195, 145, 220), step_Horizontal, new_step_Vertical, 0, "X");
    }
    step_Horizontal = step_Horizontal + 30;
    ptrReportColumns = ptrReportColumns->next;
  }

  al_flip_display();

  //Clean up resources and exit with appropriate code

  endGame();
  freeMartians();//Free source thread (Martian List)
  clearListMartians();// Delete Martian List

  al_destroy_bitmap(martian_img);
  al_destroy_bitmap(maze_img);
  al_destroy_timer(timer);
  al_destroy_display(display);
  al_destroy_event_queue(event_queue);
  return code;
}

#define INCVAL (0.1f)
int st = 1;
int go = 0;
#define RESTVAL (0.01f)
#define UNKNOWN_ERROR (0)
static void *Func_Thread(ALLEGRO_THREAD *thr, void *arg){
  node *_martianData = arg;
  al_lock_mutex(_martianData->mutex);
  char mw = _martianData->goTo;
  al_broadcast_cond(_martianData->cond);
  al_unlock_mutex(_martianData->mutex);

  while (!al_get_thread_should_stop(thr)){
    if (_martianData->isActive){
      al_lock_mutex(_martianData->mutex);
      srand(time(0));
      go = (rand() % 4) + 1;

      if (go == 1 && MAZE_MATRIX_BIG[_martianData->row - 1][_martianData->column] == 1 && MAZE_MATRIX_BIG[_martianData->row - 1][_martianData->column + 31] == 1){//1: Espacio para caminar
        _martianData->row -= st; // U: Up
      }
      else if (go == 2 && MAZE_MATRIX_BIG[_martianData->row][_martianData->column + 32] == 1 && MAZE_MATRIX_BIG[_martianData->row + 31][_martianData->column + 32] == 1){//1: Espacio para caminar
        _martianData->column += st; //  R: Right
      }
      else if (go == 3 && MAZE_MATRIX_BIG[_martianData->row + 32][_martianData->column] == 1 && MAZE_MATRIX_BIG[_martianData->row + 32][_martianData->column + 31] == 1){//1: Espacio para caminar
        _martianData->row += st; //  D:Down
      }
      else if (go == 4 && MAZE_MATRIX_BIG[_martianData->row][_martianData->column - 1] == 1 && MAZE_MATRIX_BIG[_martianData->row + 31][_martianData->column - 1] == 1){//1: Espacio para caminar
        _martianData->column -= st; // L:Left
      }
      else{
      }
      al_unlock_mutex(_martianData->mutex);
      al_rest(RESTVAL);
    }
  }
  return NULL;
}

// ---------------------------------------------------------------------------
// EVENT HANDLER 
// ---------------------------------------------------------------------------
code HandleEvent(ALLEGRO_EVENT ev){
  switch (ev.type){
  case ALLEGRO_EVENT_TIMER:
    RedrawSetReady();
    break;

  case ALLEGRO_EVENT_DISPLAY_CLOSE:
    return EXIT_SUCCESS;

  case ALLEGRO_EVENT_KEY_DOWN:
    if (ev.keyboard.keycode == ALLEGRO_KEY_X){
      printf("¡END OF THE GAME!\n");
      printReportConsole();
      return EXIT_SUCCESS;
    }
    else if (ev.keyboard.keycode == ALLEGRO_KEY_S){
      if (__mode == 1)
        __start_auto = 1;
    }
    else if (ev.keyboard.keycode == ALLEGRO_KEY_R){
      printReportConsole();
    }
    else if (ev.keyboard.keycode == ALLEGRO_KEY_L)
      printListMartians();

    else if (ev.keyboard.keycode == ALLEGRO_KEY_T)
      length();

    else if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
    {
      new_martian = 0;
      energy = 0;
      period = 0;
    }
    else if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER)
    {
      if (new_martian == 1) // Deshabilita la creacion de marcianos
      {
        if (energy != 0 && period != 0 && energy < period)
          addMartian(energy, period);
        new_martian = 0;
        energy = 0;
        period = 0;
      }
      else //Habilita la creacion de marcianos
        new_martian = 1;
    }
    else if (new_martian == 1){
      if (ev.keyboard.keycode == ALLEGRO_KEY_E)
        energy++;
      else if (ev.keyboard.keycode == ALLEGRO_KEY_P)
        period++;
    }
    break;
  default:
    break;
  }
  return CODE_CONTINUE;
}

void RedrawDo(node *martianData, ALLEGRO_BITMAP *martian_img)
{
  al_lock_mutex(martianData->mutex);
  float X = martianData->column;
  float Y = martianData->row;
  al_unlock_mutex(martianData->mutex);
  al_draw_bitmap(martian_img, X, Y, 0);
}

// ---------------------------------------------------------------------------
//REDRAW HANDLER
// ---------------------------------------------------------------------------
bool REDRAW_IS_READY = false;
void RedrawSetReady(void) { REDRAW_IS_READY = true; }
void RedrawClearReady(void) { REDRAW_IS_READY = false; }
bool RedrawIsReady(void){

  switch (REDRAW_IS_READY){
  case true:
    RedrawClearReady();
    return true;
  default:
    return false;
  }
}//End Project