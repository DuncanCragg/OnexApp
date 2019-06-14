
extern char* dragPathId;

void kill_drag();
void track_drag(char* pathId, bool twodimensions);
void set_drag_scroll(char* path);


extern int   linkDirection;
extern char* linkFrom;
extern char* linkTo;
extern ImVec2 linkToPos;
extern ImVec2 linkFromPos;

void track_link(bool from, char* path, int width, int height);
void draw_link();


bool is_open(char* path);
void toggle_open(char* path);
void close_all_starting(char* prefix);

void show_keyboard(float multy);
void hide_keyboard();

