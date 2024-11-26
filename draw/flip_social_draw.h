#ifndef FLIP_SOCIAL_DRAW_H
#define FLIP_SOCIAL_DRAW_H
#include <flip_social.h>
#include <flip_storage/flip_social_storage.h>
#include <callback/flip_social_callback.h>
#include <friends/flip_social_friends.h>

typedef enum
{
    ActionNone,
    ActionBack,
    ActionNext,
    ActionPrev,
    ActionFlip,
} Action;

extern Action action;

bool flip_social_board_is_active(Canvas *canvas);

void flip_social_handle_error(Canvas *canvas);

void on_input(const void *event, void *ctx);
// Function to draw the message on the canvas with word wrapping
void draw_user_message(Canvas *canvas, const char *user_message, int x, int y);

void flip_social_callback_draw_compose(Canvas *canvas, void *model);
// function to draw the dialog canvas
void flip_social_canvas_draw_message(Canvas *canvas, char *user_username, char *user_message, bool is_flipped, bool show_prev, bool show_next, int flip_count);
// Callback function to handle the feed dialog
void flip_social_callback_draw_feed(Canvas *canvas, void *model);

// function to draw the dialog canvas
void flip_social_canvas_draw_explore(Canvas *canvas, char *user_username, char *content);

// Callback function to handle the explore dialog
void flip_social_callback_draw_explore(Canvas *canvas, void *model);

// Callback function to handle the friends dialog
void flip_social_callback_draw_friends(Canvas *canvas, void *model);

void flip_social_canvas_draw_user_message(Canvas *canvas, char *user_username, char *user_message, bool show_prev, bool show_next);
// Callback function to handle the messages dialog
void flip_social_callback_draw_messages(Canvas *canvas, void *model);

#endif