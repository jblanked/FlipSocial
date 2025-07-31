#pragma once
#include "easy_flipper/easy_flipper.h"
#include "loading/loading.hpp"

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
    UserInfoCredentialsMissing = -1, // Credentials missing
    UserInfoSuccess = 0,             // User info fetched successfully
    UserInfoRequestError = 1,        // Request error
    UserInfoNotStarted = 2,          // User info request not started
    UserInfoWaiting = 3,             // Waiting for response
    UserInfoParseError = 4,          // Error parsing user info
} UserInfoStatus;

typedef enum
{
    RequestTypeLogin = 0,        // Request login (login the user)
    RequestTypeRegistration = 1, // Request registration (register the user)
    RequestTypeUserInfo = 2,     // Request user info (fetch user info)
    RequestTypeFeed = 3,         // Request feed (fetch user feed)
    RequestTypeMessages = 4,     // Request messages (fetch user messages)
} RequestType;

class FlipSocialApp;

class FlipSocialRun
{
    void *appContext;                      // reference to the app context
    SocialView currentMenuIndex;           // current menu index
    SocialView currentView;                // current view of the social run
    bool inputHeld;                        // flag to check if input is held
    bool isLoggedIn;                       // flag to check if user is logged in
    InputKey lastInput;                    // last input key pressed
    std::unique_ptr<Loading> loading;      // loading animation instance
    LoginStatus loginStatus;               // current login status
    RegistrationStatus registrationStatus; // current registration status
    bool shouldDebounce;                   // flag to debounce input
    bool shouldReturnToMenu;               // Flag to signal return to menu
    UserInfoStatus userInfoStatus;         // current user info status
    //
    void debounceInput();                      // debounce input to prevent multiple triggers
    void drawLoginView(Canvas *canvas);        // draw the login view
    void drawMainMenuView(Canvas *canvas);     // draw the main menu view
    void drawRegistrationView(Canvas *canvas); // draw the registration view
    void drawUserInfoView(Canvas *canvas);     // draw the user info view
    bool httpRequestIsFinished();              // check if the HTTP request is finished
    void userRequest(RequestType requestType); // Send a user request to the server based on the request type
public:
    FlipSocialRun(void *appContext);
    ~FlipSocialRun();
    //
    void drawMenu(Canvas *canvas, uint8_t selectedIndex, const char **menuItems, uint8_t menuCount); // Generic menu drawer
    bool isActive() const { return shouldReturnToMenu == false; }                                    // Check if the run is active
    void updateDraw(Canvas *canvas);                                                                 // update and draw the run
    void updateInput(InputEvent *event);                                                             // update input for the run
};
