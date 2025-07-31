#pragma once
#include "easy_flipper/easy_flipper.h"

typedef enum
{
    SocialViewMenu = -1,        // main menu view
    SocialViewFeed = 0,         // feed view
    SocialViewMessages = 1,     // messages view
    SocialViewProfile = 2,      // profile view
    SocialViewLogin = 3,        // login view
    SocialViewRegistration = 4, // registration view
} SocialView;

typedef enum
{
    LoginCredentialsMissing = -1, // Credentials missing
    LoginSuccess = 0,             // Login successful
    LoginUserNotFound = 1,        // User not found
    LoginWrongPassword = 2,       // Wrong password
    LoginWaiting = 3,             // Waiting for response
    LoginNotStarted = 4,          // Login not started
    LoginRequestError = 5,        // Request error
} LoginStatus;

typedef enum
{
    RegistrationCredentialsMissing = -1, // Credentials missing
    RegistrationSuccess = 0,             // Registration successful
    RegistrationUserExists = 1,          // User already exists
    RegistrationRequestError = 2,        // Request error
    RegistrationNotStarted = 3,          // Registration not started
    RegistrationWaiting = 4,             // Waiting for response
} RegistrationStatus;

typedef enum
{
    RequestTypeLogin = 0,        // Request login (login the user)
    RequestTypeRegistration = 1, // Request registration (register the user)
    RequestTypeUserInfo = 2,     // Request user info (fetch user info)
} RequestType;

class FlipSocialApp;

class FlipSocialRun
{
    void *appContext;            // reference to the app context
    SocialView currentMenuIndex; // current menu index
    SocialView currentView;      // current view of the social run
    bool inputHeld;              // flag to check if input is held
    InputKey lastInput;          // last input key pressed
    bool shouldDebounce;         // flag to debounce input
    bool shouldReturnToMenu;     // Flag to signal return to menu
    //
    void debounceInput(); // debounce input to prevent multiple triggers
public:
    FlipSocialRun(void *appContext);
    ~FlipSocialRun();
    //
    void drawMenu(Canvas *canvas, uint8_t selectedIndex, const char **menuItems, uint8_t menuCount); // Generic menu drawer
    bool isActive() const { return shouldReturnToMenu == false; }                                    // Check if the run is active
    void updateDraw(Canvas *canvas);                                                                 // update and draw the run
    void updateInput(InputEvent *event);                                                             // update input for the run
};
