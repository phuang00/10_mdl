/*========== my_main.c ==========

  This is the only file you need to modify in order
  to get a working mdl project (for now).

  my_main.c will serve as the interpreter for mdl.
  When an mdl script goes through a lexer and parser,
  the resulting operations will be in the array op[].

  Your job is to go through each entry in op and perform
  the required action from the list below:

  push: push a new origin matrix onto the origin stack

  pop: remove the top matrix on the origin stack

  move/scale/rotate: create a transformation matrix
                     based on the provided values, then
                     multiply the current top of the
                     origins stack by it.

  box/sphere/torus: create a solid object based on the
                    provided values. Store that in a
                    temporary matrix, multiply it by the
                    current top of the origins stack, then
                    call draw_polygons.

  line: create a line based on the provided values. Store
        that in a temporary matrix, multiply it by the
        current top of the origins stack, then call draw_lines.

  save: call save_extension with the provided filename

  display: view the screen
  =========================*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "parser.h"
#include "symtab.h"
#include "y.tab.h"

#include "matrix.h"
#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "stack.h"
#include "gmath.h"

void my_main() {

  int i;
  struct matrix *tmp;
  struct matrix *trans;
  struct stack *systems;
  screen t;
  zbuffer zb;
  color g;
  double step_3d = 100;
  double theta;

  //Lighting values here for easy access
  color ambient;
  ambient.red = 50;
  ambient.green = 50;
  ambient.blue = 50;

  double light[2][3];
  light[LOCATION][0] = 0.5;
  light[LOCATION][1] = 0.75;
  light[LOCATION][2] = 1;

  light[COLOR][RED] = 255;
  light[COLOR][GREEN] = 255;
  light[COLOR][BLUE] = 255;

  double view[3];
  view[0] = 0;
  view[1] = 0;
  view[2] = 1;

  //default reflective constants if none are set in script file
  struct constants white;
  white.r[AMBIENT_R] = 0.1;
  white.g[AMBIENT_R] = 0.1;
  white.b[AMBIENT_R] = 0.1;

  white.r[DIFFUSE_R] = 0.5;
  white.g[DIFFUSE_R] = 0.5;
  white.b[DIFFUSE_R] = 0.5;

  white.r[SPECULAR_R] = 0.5;
  white.g[SPECULAR_R] = 0.5;
  white.b[SPECULAR_R] = 0.5;

  //constants are a pointer in symtab, using one here for consistency
  struct constants *reflect;
  reflect = &white;

  systems = new_stack();
  tmp = new_matrix(4, 1000);
  clear_screen( t );
  clear_zbuffer(zb);
  g.red = 0;
  g.green = 0;
  g.blue = 0;

  print_symtab();
  for (i=0;i<lastop;i++) {

    printf("%d: ",i);
    switch (op[i].opcode)
      {
      case LIGHT:
        printf("Light: %s at: %6.2f %6.2f %6.2f",
               op[i].op.light.p->name,
               op[i].op.light.c[0], op[i].op.light.c[1],
               op[i].op.light.c[2]);
        break;
      case AMBIENT:
        printf("Ambient: %6.2f %6.2f %6.2f",
               op[i].op.ambient.c[0],
               op[i].op.ambient.c[1],
               op[i].op.ambient.c[2]);
        break;

      case CONSTANTS:
        printf("Constants: %s",op[i].op.constants.p->name);
        break;
      case SAVE_COORDS:
        printf("Save Coords: %s",op[i].op.save_coordinate_system.p->name);
        break;
      case CAMERA:
        printf("Camera: eye: %6.2f %6.2f %6.2f\taim: %6.2f %6.2f %6.2f",
               op[i].op.camera.eye[0], op[i].op.camera.eye[1],
               op[i].op.camera.eye[2],
               op[i].op.camera.aim[0], op[i].op.camera.aim[1],
               op[i].op.camera.aim[2]);

        break;
      case SPHERE:
        printf("Sphere: %6.2f %6.2f %6.2f r=%6.2f",
               op[i].op.sphere.d[0],op[i].op.sphere.d[1],
               op[i].op.sphere.d[2],
               op[i].op.sphere.r);
        add_sphere(tmp, op[i].op.sphere.d[0],op[i].op.sphere.d[1], op[i].op.sphere.d[2], op[i].op.sphere.r, step_3d);
        matrix_mult(peek(systems), tmp);
        if (op[i].op.sphere.constants != NULL)
          {
            printf("\tconstants: %s",op[i].op.sphere.constants->name);
            struct constants *c = op[i].op.sphere.constants->s.c;
            g.red = c->red;
            g.blue = c->blue;
            g.green = c->green;
            draw_polygons(tmp, t, zb, view, light, g, c);
          }
        else
          {
            draw_polygons(tmp, t, zb, view, light, ambient, reflect);
          }
        if (op[i].op.sphere.cs != NULL)
          {
            printf("\tcs: %s",op[i].op.sphere.cs->name);
          }
        tmp->lastcol = 0;
        break;
      case TORUS:
        printf("Torus: %6.2f %6.2f %6.2f r0=%6.2f r1=%6.2f",
               op[i].op.torus.d[0],op[i].op.torus.d[1],
               op[i].op.torus.d[2],
               op[i].op.torus.r0,op[i].op.torus.r1);
        add_torus(tmp, op[i].op.torus.d[0],op[i].op.torus.d[1], op[i].op.torus.d[2], op[i].op.torus.r0,op[i].op.torus.r1, step_3d);
        matrix_mult(peek(systems), tmp);
        if (op[i].op.torus.constants != NULL)
          {
            printf("\tconstants: %s",op[i].op.torus.constants->name);
            struct constants *c = op[i].op.torus.constants->s.c;
            g.red = c->red;
            g.blue = c->blue;
            g.green = c->green;
            draw_polygons(tmp, t, zb, view, light, g, c);
          }
        else
          {
            draw_polygons(tmp, t, zb, view, light, ambient, reflect);
          }
        if (op[i].op.torus.cs != NULL)
          {
            printf("\tcs: %s",op[i].op.torus.cs->name);
          }
        tmp->lastcol = 0;
        break;
      case BOX:
        printf("Box: d0: %6.2f %6.2f %6.2f d1: %6.2f %6.2f %6.2f",
               op[i].op.box.d0[0],op[i].op.box.d0[1],
               op[i].op.box.d0[2],
               op[i].op.box.d1[0],op[i].op.box.d1[1],
               op[i].op.box.d1[2]);
        add_box(tmp, op[i].op.box.d0[0],op[i].op.box.d0[1], op[i].op.box.d0[2], op[i].op.box.d1[0],op[i].op.box.d1[1], op[i].op.box.d1[2]);
        matrix_mult(peek(systems), tmp);
        if (op[i].op.box.constants != NULL)
          {
            printf("\tconstants: %s",op[i].op.box.constants->name);
            struct constants *c = op[i].op.box.constants->s.c;
            g.red = c->red;
            g.blue = c->blue;
            g.green = c->green;
            draw_polygons(tmp, t, zb, view, light, g, c);
          }
        else
          {
            draw_polygons(tmp, t, zb, view, light, ambient, reflect);
          }
        if (op[i].op.box.cs != NULL)
          {
            printf("\tcs: %s",op[i].op.box.cs->name);
          }
        tmp->lastcol = 0;
        break;
      case LINE:
        printf("Line: from: %6.2f %6.2f %6.2f to: %6.2f %6.2f %6.2f",
               op[i].op.line.p0[0],op[i].op.line.p0[1],
               op[i].op.line.p0[1],
               op[i].op.line.p1[0],op[i].op.line.p1[1],
               op[i].op.line.p1[1]);
        add_edge(tmp, op[i].op.line.p0[0],op[i].op.line.p0[1], op[i].op.line.p0[1], op[i].op.line.p1[0],op[i].op.line.p1[1], op[i].op.line.p1[1]);
        matrix_mult(peek(systems), tmp);
        draw_lines(tmp, t, zb, g);
        tmp->lastcol = 0;
        if (op[i].op.line.constants != NULL)
          {
            printf("\n\tConstants: %s",op[i].op.line.constants->name);
          }
        if (op[i].op.line.cs0 != NULL)
          {
            printf("\n\tCS0: %s",op[i].op.line.cs0->name);
          }
        if (op[i].op.line.cs1 != NULL)
          {
            printf("\n\tCS1: %s",op[i].op.line.cs1->name);
          }
        break;
      case MESH:
        printf("Mesh: filename: %s",op[i].op.mesh.name);
        if (op[i].op.mesh.constants != NULL)
          {
            printf("\tconstants: %s",op[i].op.mesh.constants->name);
          }
        break;
      case SET:
        printf("Set: %s %6.2f",
               op[i].op.set.p->name,
               op[i].op.set.p->s.value);
        break;
      case MOVE:
        printf("Move: %6.2f %6.2f %6.2f",
               op[i].op.move.d[0],op[i].op.move.d[1],
               op[i].op.move.d[2]);
        trans = make_translate(op[i].op.move.d[0],op[i].op.move.d[1], op[i].op.move.d[2]);
        matrix_mult(peek(systems), trans);
        copy_matrix(trans, peek(systems));
        if (op[i].op.move.p != NULL)
          {
            printf("\tknob: %s",op[i].op.move.p->name);
          }
        break;
      case SCALE:
        printf("Scale: %6.2f %6.2f %6.2f",
               op[i].op.scale.d[0],op[i].op.scale.d[1],
               op[i].op.scale.d[2]);
        trans = make_scale(op[i].op.scale.d[0],op[i].op.scale.d[1], op[i].op.scale.d[2]);
        matrix_mult(peek(systems), trans);
        copy_matrix(trans, peek(systems));
        if (op[i].op.scale.p != NULL)
          {
            printf("\tknob: %s",op[i].op.scale.p->name);
          }
        break;
      case ROTATE:
        printf("Rotate: axis: %6.2f degrees: %6.2f",
               op[i].op.rotate.axis,
               op[i].op.rotate.degrees);
        double axis = op[i].op.rotate.axis;
        theta = op[i].op.rotate.degrees * (M_PI / 180);
        if ( axis == 0 )
          trans = make_rotX( theta );
        else if ( axis == 1 )
          trans = make_rotY( theta );
        else
          trans = make_rotZ( theta );
        matrix_mult(peek(systems), trans);
        copy_matrix(trans, peek(systems));
        if (op[i].op.rotate.p != NULL)
          {
            printf("\tknob: %s",op[i].op.rotate.p->name);
          }
        break;
      case BASENAME:
        printf("Basename: %s",op[i].op.basename.p->name);
        break;
      case SAVE_KNOBS:
        printf("Save knobs: %s",op[i].op.save_knobs.p->name);
        break;
      case TWEEN:
        printf("Tween: %4.0f %4.0f, %s %s",
               op[i].op.tween.start_frame,
               op[i].op.tween.end_frame,
               op[i].op.tween.knob_list0->name,
               op[i].op.tween.knob_list1->name);
        break;
      case FRAMES:
        printf("Num frames: %4.0f",op[i].op.frames.num_frames);
        break;
      case VARY:
        printf("Vary: %4.0f %4.0f, %4.0f %4.0f",
               op[i].op.vary.start_frame,
               op[i].op.vary.end_frame,
               op[i].op.vary.start_val,
               op[i].op.vary.end_val);
        break;
      case PUSH:
        printf("Push");
        push(systems);
        break;
      case POP:
        printf("Pop");
        pop(systems);
        break;
      case GENERATE_RAYFILES:
        printf("Generate Ray Files");
        break;
      case SAVE:
        printf("Save: %s",op[i].op.save.p->name);
        save_extension(t, op[i].op.save.p->name);
        break;
      case SHADING:
        printf("Shading: %s",op[i].op.shading.p->name);
        break;
      case SETKNOBS:
        printf("Setknobs: %f",op[i].op.setknobs.value);
        break;
      case FOCAL:
        printf("Focal: %f",op[i].op.focal.value);
        break;
      case DISPLAY:
        printf("Display");
        display(t);
        break;
      }

    printf("\n");
  }
}
