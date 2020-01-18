#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define STEP 100
#define FLYINGPIG_ID 1
#define TIMER_ID 0
#define TIMER_INTERVAL 20

//struktura za cuvanje modela
typedef struct Vertex
{
    double position[3];
    double texcoord[2];
    double normal[3];
} Vertex;

//struktura za cuvanje podataka iz .obj fajla
typedef struct VertRef
{
    int v, vt, vn;
} VertRef;

//promenljive za pomeranje kamere misom
static int mouse_x, mouse_y;
static int window_width, window_height;
static float matrix[16];
static int xListA[7];
static int zListA[7];
static int xListB[7];
static int zListB[7];
static float zPlaneA = -15;
static float zPlaneB = -45;
static int gameEnded = 0;

static void on_keyboard(unsigned char key, int x, int y);
static void on_mouse(int button, int state, int x, int y);
static void on_motion(int x, int y);
static void on_reshape(int width, int height);
static void on_display(void);
static void on_timer(int value);

static void draw_axis(float len);
static void draw_pig(void);
static void draw_planeA(void);
static void draw_planeB(void);
static void draw_obstaclesA(void);
static void draw_obstaclesB(void);

static void generate_random_poitionA(void);
static void regenerate_random_poitionA(void);
static void regenerate_random_poitionB(void);


static float distance(float x, float y, float z);
static void collision(void);

Vertex* LoadObj(FILE * file, int id);

static Vertex *model;
static int model_size = 0;
static int animation_ongoing = 0;
static int xPig = 0, yPig = 0, zPig = 0;

int main(int argc, char **argv)
{
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

 
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(argv[0]);
    
    // srand(time(NULL));
    generate_random_poitionA();
    

    glutKeyboardFunc(on_keyboard);
    glutReshapeFunc(on_reshape);
    glutDisplayFunc(on_display);
    glutMouseFunc(on_mouse);
    glutMotionFunc(on_motion);
    
    mouse_x = 0;
    mouse_y = 0;
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);

    glClearColor(0.75, 0.75, 0.75, 0);
    glEnable(GL_DEPTH_TEST);
    glLineWidth(2);

    FILE * fileP = fopen("./models/Pig.obj", "r");
    if( fileP == NULL ){
        printf("Impossible to open the file !\n");
        return false;
    }
    model = LoadObj(fileP, FLYINGPIG_ID);
   
    glutMainLoop();

    return 0;
}

void draw_axis(float len) {
    glDisable(GL_LIGHTING);
    
    glBegin(GL_LINES);
        glColor3f(1,0,0);
        glVertex3f(-len,0,0);
        glVertex3f(len,0,0);

        glColor3f(0,1,0);
        glVertex3f(0,-len,0);
        glVertex3f(0,len,0);

        glColor3f(0,0,1);
        glVertex3f(0,0,-len);
        glVertex3f(0,0,len);
    glEnd();

    glEnable(GL_LIGHTING);
}

static void draw_obstaclesA(void) {
    
    int num = 7;
    for(int i=0; i<num; i++){
    
        glPushMatrix();
        glColor3f(1,0,0);
        glTranslatef(xListA[i], 0, zListA[i]);
        glScalef(1,2,1);
        glutSolidCube(1);
        glPopMatrix();
    }
    
}

static void draw_obstaclesB(void) {
    
    int num = 7;
    for(int i=0; i<num; i++){
    
        glPushMatrix();
        glColor3f(1,0,0);
        glTranslatef(xListB[i], 0, zListB[i]);
        glScalef(1,2,1);
        glutSolidCube(1);
        glPopMatrix();
    }
    
}

static void draw_planeB(void) {
    glDisable(GL_LIGHTING);
    
    glTranslatef(0,0, zPlaneB);
    draw_obstaclesB();
    glTranslatef(0,-0.5,0);
    glScalef(11, 1, 30);
    glColor3f(0.5,0.35,0.05);
    glutSolidCube(1);
    glEnable(GL_LIGHTING);
}

static void draw_planeA(void) {
    glDisable(GL_LIGHTING);
    
    glTranslatef(0,0, zPlaneA);
    draw_obstaclesA();
    glTranslatef(0,-0.5,0);
    glScalef(11, 1, 30);
    glColor3f(0.5,0.35,0.05);
    glutSolidCube(1);
    glEnable(GL_LIGHTING);
}

static void draw_pig(void)
{
    glDisable(GL_LIGHTING);
    GLfloat ambient_coeffs[] = { 0.4, 0, 0.7, 1 };
    GLfloat diffuse_coeffs[] = { 0.4, 0, 0.7, 1 };
    GLfloat specular_coeffs[] = { 0.4, 0, 0.7, 1};
    
    GLfloat shininess = 30;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient_coeffs);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse_coeffs);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular_coeffs);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    
    glTranslatef(1.3, 0,0);
    glRotatef(180, 0, 1, 0);
    glRotatef(90, -1, 0, 0);
    // glScalef(.2, .2, .2);
    /*
    Model ucitavamo tako sto inicijalizujemo glBegin na GL_TRIANGLES
    prolazimo petljom kroz niz tipa Vertex i citamo redom potrebne podatke
    u glnormal3f stavaljmo odgovarajuce koordinate (0=x, 1=y, 2=z)
    isto i za glVertex3f
    posle toga zavrsavamo sa glEnd
    Dalje svaki sledeci model ucitavamo po istom principu
    */
    
    glBegin(GL_TRIANGLES);
        glColor3f(1,0,1);
        for(int i=0; i<model_size; i++){
                glNormal3f(model[i].normal[0], model[i].normal[1], model[i].normal[2]);
                glVertex3f(model[i].position[0], model[i].position[1], model[i].position[2]);
        } 
    glEnd();
    glEnable(GL_LIGHTING);
}

static void on_keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    
    case 'g':
    case 'G':
        if (!animation_ongoing && !gameEnded) {
            glutTimerFunc(TIMER_INTERVAL, on_timer, TIMER_ID);
            animation_ongoing = 1;
        }
        break;

    case 's':
    case 'S':
        animation_ongoing = 0;
        break;
    case 'a':
    case 'A':
        if (xPig > -5)
            xPig -=1;
        glutPostRedisplay();
        break;
    case 'd':
    case 'D':
        if(xPig < 5)
            xPig +=1;
        glutPostRedisplay();
        break;
    case 'r':
    case 'R':
        zPlaneA = -15;
        zPlaneB = -45;
        animation_ongoing = 0;
        gameEnded = 0;
        xPig = 0;
        regenerate_random_poitionA();
        regenerate_random_poitionB();
        glutPostRedisplay();
        break;
    case 27:
        exit(0);
        break;
    }
}

static void on_timer(int value)
{
    
    if (value != TIMER_ID)
        return;

    zPlaneA = zPlaneA + 0.5f;
    zPlaneB = zPlaneB + 0.5f;
    collision();
    if (zPlaneA - 15 > 5)
    {
        regenerate_random_poitionA();
        zPlaneA = -39;
    }

    if (zPlaneB - 15 > 5)
{       
        
        regenerate_random_poitionB();
        zPlaneB = -39;
    }
    
    glutPostRedisplay();
    if (animation_ongoing) {
        glutTimerFunc(TIMER_INTERVAL, on_timer, TIMER_ID);
    }
}

static void on_reshape(int width, int height)
{
    
    window_width = width;
    window_height = height;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (float) width / height, 1, 35);
}

static void on_display(void)
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(xPig, yPig*1.0 + 2.1, zPig + 4, xPig, yPig, zPig, 0, 1, 0);
    glMultMatrixf(matrix);
    
    draw_axis(100);

    glPushMatrix();
        draw_planeB();
    glPopMatrix();

    glPushMatrix();
        draw_planeA();
    glPopMatrix();

    glPushMatrix();
        glTranslatef(xPig, yPig, zPig);
        draw_pig();
    glPopMatrix();

    glutSwapBuffers();
}

Vertex* LoadObj(FILE * file, int id){
    //na pocetku pravimo prostor na memoriji za model
    int verts_count = 0;
    int verts_count_of = STEP;
    Vertex *verts = malloc(verts_count_of * sizeof(Vertex));
    
    //pozicija
    int num_of_pos = STEP;
    double **positions = malloc(num_of_pos * sizeof(double*));
    for(int i=0; i<num_of_pos; i++){
        positions[i] = malloc(3 * sizeof(double));
    }
    positions[0][0] = 0;
    positions[0][1] = 0;
    positions[0][2] = 0;

    //teksture
    int num_of_tc = STEP;
    double **texcoords = malloc(num_of_tc * sizeof(double*));
    for(int i=0; i<num_of_tc; i++){
        texcoords[i] = malloc(3*sizeof(double));
    }
    texcoords[0][0] = 0;
    texcoords[0][1] = 0;
    texcoords[0][2] = 0;

    //normale
    int num_of_n = STEP;
    double **normals = malloc(num_of_n * sizeof(double*));
    for(int i=0; i<num_of_n; i++){
        normals[i] = malloc(3*sizeof(double));
    }
    normals[0][0] = 0;
    normals[0][1] = 0;
    normals[0][2] = 0;

    char *line = NULL;
    size_t len = 0;
    size_t read = 0;
    int countPos = 1;
    int countTC = 1;
    int countN = 1;
    //citamo liniju po liniju i u odnosu na odgovarajuce slovo stavljamo koordinate u odgovarajucu matricu
    while((read = getline(&line, &len, file)) != -1){
        char type[5];
        sscanf(line, "%s ", type);
        if(strcmp(type, "v") == 0){
            //koordinate za poziciju
            double x = 0, y = 0, z = 0;
            sscanf(line, "v %lf %lf %lf",   &x, &y, &z);
            if(countPos >= num_of_pos){
                num_of_pos += STEP;
                positions = realloc(positions, num_of_pos * sizeof(double*));
                for(int i=countPos; i<num_of_pos; i++)
                    positions[i] = malloc(3 * sizeof(double));
            }
            positions[countPos][0] = x;
            positions[countPos][1] = y;
            positions[countPos][2] = z;
            countPos += 1;
        }
        
        if(strcmp(type, "vt") == 0){
            //koordinate za teksture
            double u = 0, v = 0, t = 0;
            sscanf(line, "vt %lf %lf %lf", &u, &v, &t);
            if(countTC >= num_of_tc){
                num_of_tc += STEP;
                texcoords = realloc(texcoords, num_of_tc * sizeof(double*));
                for(int i=countTC; i<num_of_tc; i++)
                    texcoords[i] = malloc(3*sizeof(double));
            }
            texcoords[countTC][0] = u;
            texcoords[countTC][1] = v;
            texcoords[countTC][2] = t;
            countTC += 1;
        }

        if(strcmp(type, "vn") == 0){
            //koordinate za normale
            double i = 0, j = 0, k = 0;
            sscanf(line, "vn %lf %lf %lf", &i, &j, &k);
            if(countN >= num_of_n){
                num_of_n += STEP;
                normals = realloc(normals, num_of_n * sizeof(double*));
                for(int i=countN; i<num_of_n; i++)
                    normals[i] = malloc(3*sizeof(double));
            }
            normals[countN][0] = i;
            normals[countN][1] = j;
            normals[countN][2] = k;
            countN += 1;
        }

        if(strcmp(type, "f") == 0){
            //koordinate koje nam govore kako su rasporedjene koordinate pozicija, tekstura i normala
            int ref_step = STEP;
            VertRef *refs = malloc(ref_step * sizeof(VertRef));
            char a[256];
            char *newF = strchr(line, 'f') + 2;
            
            int offset = 0;
            int ref_count = 0;
            while(sscanf(newF, " %s%n", a, &offset) == 1){
                char *vta = strchr(a, '/')+1;
                char *vna = strchr(vta, '/')+1;
                a[strlen(a) - strlen(vta)-1] = '\0';
                if(vta[0] == '/'){
                    vta = "0";
                }
                
                
                newF += offset;
                if(ref_count >= ref_step){
                    ref_step += STEP;
                    refs = realloc(refs, ref_step*sizeof(VertRef));
                }
                refs[ref_count].v = atoi(a);
                refs[ref_count].vn = atoi(vna);
                refs[ref_count].vt = atoi(vta);
                ref_count += 1;
            }
            //redjanju tacka tako da budu dobre za triangulaciju
            for(int i=1; i+1 < ref_count; i++){
                const VertRef *p[3] = {&refs[0], &refs[i], &refs[i+1]};
                double U[3] ={0};
                U[0] = positions[ p[1]->v ][0] - positions[ p[0]->v ][0];
                U[1] = positions[ p[1]->v ][1] - positions[ p[0]->v ][1];
                U[2] = positions[ p[1]->v ][2] - positions[ p[0]->v ][2];
                double V[3] ={0};
                V[0] = positions[ p[2]->v ][0] - positions[ p[0]->v ][0];
                V[1] = positions[ p[2]->v ][1] - positions[ p[0]->v ][1];
                V[2] = positions[ p[2]->v ][2] - positions[ p[0]->v ][2];
                double N[3] = {0};

                N[0] = U[1]*V[2] - U[2]*V[1];
                N[1] = U[2]*V[0] - U[0]*V[2];
                N[2] = U[0]*V[1] - U[1]*V[0];

                double w = sqrt(N[0]*N[0] + N[1]*N[1] + N[2]*N[2]);
                N[0] /= w;
                N[1] /= w;
                N[2] /= w;
                for(int j=0; j<3; j++){
                    Vertex vert;

                    vert.position[0] = positions[ p[j]->v ][0];
                    vert.position[1] = positions[ p[j]->v ][1];
                    vert.position[2] = positions[ p[j]->v ][2];

                    vert.texcoord[0] = texcoords[ p[j]->vt ][0];
                    vert.texcoord[1] = texcoords[ p[j]->vt ][1];
                    if(p[j]->vn != 0){
                        vert.normal[0] = normals[ p[j]->vn ][0];
                        vert.normal[1] = normals[ p[j]->vn ][1];
                        vert.normal[2] = normals[ p[j]->vn ][2];
                    } else {
                        vert.normal[0] = N[0];
                        vert.normal[1] = N[1];
                        vert.normal[2] = N[2];
                    }
                    if(verts_count >= verts_count_of){
                        verts_count_of += STEP;
                        verts = realloc(verts, verts_count_of*sizeof(Vertex));
                    }
                    verts[verts_count].position[0] = vert.position[0];
                    verts[verts_count].position[1] = vert.position[1];
                    verts[verts_count].position[2] = vert.position[2];

                    verts[verts_count].normal[0] = vert.normal[0];
                    verts[verts_count].normal[1] = vert.normal[1];
                    verts[verts_count].normal[2] = vert.normal[2];

                    verts[verts_count].texcoord[0] = vert.texcoord[50];
                    verts[verts_count].texcoord[1] = vert.texcoord[1];
                    verts_count += 1;
                }
            }
            free(refs);
        }

    }
    //pamcenje duzine niza u odnosu na odgovarajuci id
    if(id == FLYINGPIG_ID)
        model_size = verts_count;    

    //oslobadjanje memorije
    for(int i=0; i<countPos; i++)
        free(positions[i]);
    free(positions);

    for(int i=0; i<countN; i++)
        free(normals[i]);
    free(normals);

    for(int i=0; i<countTC; i++)
        free(texcoords[i]);
    free(texcoords);

    return verts;
}

static void on_mouse(int button, int state, int x, int y)
{
    mouse_x = x;
    mouse_y = y;
}
//funkcija za pokret
static void on_motion(int x, int y)
{
    int delta_x, delta_y;

    delta_x = x - mouse_x;
    delta_y = y - mouse_y;

    
    mouse_x = x;
    mouse_y = y;

    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
        glLoadIdentity();
        glRotatef(180 * (float) delta_x / window_width, 0, 1, 0);
        glRotatef(180 * (float) delta_y / window_height, 1, 0, 0);
        glMultMatrixf(matrix);

        glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
    glPopMatrix();

    
    glutPostRedisplay();
}

static void generate_random_poitionA(void) {
    int num = 7;
    for(int i=0; i<num; i++){
        int v1 = rand() % 10;
        // int v2 = rand() % 10;
        zListA[i] = rand() % 15;
        xListA[i] = rand() % 6;
        zListB[i] = rand() % 15;
        xListB[i] = rand() % 6;
        if(v1 > 4){
            zListB[i] *= -1;
            zListA[i] *= -1;
        }
        if(i%2 == 0){
            xListB[i] *= -1;
            xListA[i] *= -1;
        }
        

    }
}

static void regenerate_random_poitionA(void) {
    int num = 7;
    for(int i=0; i<num; i++){
        
        int v1 = rand() % 10;
        zListA[i] = rand() % 15;
        xListA[i] = rand() % 6;
        if(v1 > 4){
            zListA[i] *= -1;
        }
        if(i%2 == 0){
            xListA[i] *= -1;
        }
        

    }
}

static void regenerate_random_poitionB(void) {
    int num = 7;
    
    for(int i=0; i<num; i++){
        int v1 = rand() % 10;
        zListB[i] = rand() % 15;
        xListB[i] = rand() % 6;
        if(v1 > 4){
            zListB[i] *= -1;
        }
        if(i%2 == 0){
            xListB[i] *= -1;
        }
        

    }
}

static float distance(float x, float y, float z)
{
    float xTmp = powf((x - xPig),2);
    float yTmp = powf((y - yPig),2);
    float zTmp = powf((z - zPig),2);

    return sqrtf(xTmp + yTmp + zTmp);
}

static void collision(void) {
    for(int i=0; i<7; i++){

        if(distance(xListA[i], 0, zListA[i] + zPlaneA) <= 0 || distance(xListB[i], 0, zListB[i] + zPlaneB) <= 0){
            animation_ongoing = 0;
            gameEnded = 1;
        }
    }
}
