#pragma once
#include "easy_flipper/easy_flipper.h"
#include "loading/loading.hpp"

#define MAX_PRE_SAVED_MESSAGES 20 // Maximum number of pre-saved messages
#define MAX_MESSAGE_LENGTH 100    // Maximum length of a message in the feed
#define MAX_EXPLORE_USERS 50      // Maximum number of users to explore
#define MAX_USER_LENGTH 32        // Maximum length of a username
#define MAX_FRIENDS 50            // Maximum number of friends
#define MAX_FEED_ITEMS 25         // Maximum number of feed items
#define MAX_MESSAGE_USERS 40      // Maximum number of users to display in the submenu
#define MAX_MESSAGES 20           // Maximum number of messages between each user

typedef enum
{
    SocialViewMenu = -1,        // main menu view
    SocialViewFeed = 0,         // feed view
    SocialViewMessageUsers = 1, // (initial) messages view
    SocialViewProfile = 2,      // profile view
    SocialViewLogin = 3,        // login view
    SocialViewRegistration = 4, // registration view
    SocialViewUserInfo = 5,     // user info view
    SocialViewMessages = 6      // messages view
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
    RequestTypeLogin = 0,            // Request login (login the user)
    RequestTypeRegistration = 1,     // Request registration (register the user)
    RequestTypeUserInfo = 2,         // Request user info (fetch user info)
    RequestTypeFeed = 3,             // Request feed (fetch user feed)
    RequestTypeFlipPost = 4,         // Request flip post (flip the current seelected post)
    RequestTypeCommentFetch = 5,     // Request comments (fetch comments for a post)
    RequestTypeMessagesUserList = 6, // Request messages (fetch list of users who sent messages)
    RequestTypeMessagesWithUser = 7, // Request messages with a specific user
} RequestType;

typedef enum
{
    ProfileElementBio,
    ProfileElementFriends,
    ProfileElementJoined,
    ProfileElementMAX
} ProfileElement;

typedef enum
{
    FeedNotStarted = 0,   // Feed not started
    FeedWaiting = 1,      // Waiting for feed response
    FeedSuccess = 2,      // Feed fetched successfully
    FeedParseError = 3,   // Error parsing feed
    FeedRequestError = 4, // Error in feed request
} FeedStatus;

typedef enum
{
    MessageUsersNotStarted = 0,   // Messages not started
    MessageUsersWaiting = 1,      // Waiting for messages response
    MessageUsersSuccess = 2,      // Messages fetched successfully
    MessageUsersParseError = 3,   // Error parsing messages
    MessageUsersRequestError = 4, // Error in messages request
} MessageUsersStatus;

typedef enum
{
    MessagesNotStarted = 0,   // Messages not started
    MessagesWaiting = 1,      // Waiting for messages response
    MessagesSuccess = 2,      // Messages fetched successfully
    MessagesParseError = 3,   // Error parsing messages
    MessagesRequestError = 4, // Error in messages request
} MessagesStatus;

class FlipSocialApp;

class FlipSocialRun
{
    void *appContext;                      // reference to the app context
    SocialView currentMenuIndex;           // current menu index
    uint8_t currentProfileElement;         // current profile element being viewed
    SocialView currentView;                // current view of the social run
    uint16_t feedItemID;                   // current feed item ID
    uint8_t feedItemIndex;                 // current feed item index
    uint8_t feedIteration;                 // current feed iteration
    FeedStatus feedStatus;                 // current feed status
    bool inputHeld;                        // flag to check if input is held
    InputKey lastInput;                    // last input key pressed
    std::unique_ptr<Loading> loading;      // loading animation instance
    LoginStatus loginStatus;               // current login status
    MessagesStatus messagesStatus;         // current messages status
    MessageUsersStatus messageUsersStatus; // current messages status
    uint8_t messagesIndex;                 // index of the message in the messages submenu
    uint8_t messageUserIndex;              // index of the user in the Message Users submenu
    RegistrationStatus registrationStatus; // current registration status
    bool shouldDebounce;                   // flag to debounce input
    bool shouldReturnToMenu;               // Flag to signal return to menu
    UserInfoStatus userInfoStatus;         // current user info status
    //
    void debounceInput();                                                                                             // debounce input to prevent multiple triggers
    void drawFeedItem(Canvas *canvas, char *username, char *message, char *flipped, char *flips, char *date_created); // draw a single feed item
    void drawFeedMessage(Canvas *canvas, const char *user_message, int x, int y);                                     // draw the feed message with wrapping
    void drawFeedView(Canvas *canvas);                                                                                // draw the feed view
    void drawLoginView(Canvas *canvas);                                                                               // draw the login view
    void drawMainMenuView(Canvas *canvas);                                                                            // draw the main menu view
    void drawMessagesView(Canvas *canvas);                                                                            // draw the messages view
    void drawMessageUsersView(Canvas *canvas);                                                                        // draw the message users view
    void drawProfileView(Canvas *canvas);                                                                             // draw the profile view
    void drawRegistrationView(Canvas *canvas);                                                                        // draw the registration view
    void drawUserInfoView(Canvas *canvas);                                                                            // draw the user info view
    void drawWrappedBio(Canvas *canvas, const char *text, uint8_t x, uint8_t y);                                      // draw wrapped text on the canvas
    bool getMessageUser(uint8_t index, char *buffer, size_t buffer_size);                                             // get the message user at the specified messageUserIndex
    bool httpRequestIsFinished();                                                                                     // check if the HTTP request is finished
    void userRequest(RequestType requestType);                                                                        // Send a user request to the server based on the request type
public:
    FlipSocialRun(void *appContext);
    ~FlipSocialRun();
    //
    void drawMenu(Canvas *canvas, uint8_t selectedIndex, const char **menuItems, uint8_t menuCount); // Generic menu drawer
    bool isActive() const { return shouldReturnToMenu == false; }                                    // Check if the run is active
    void updateDraw(Canvas *canvas);                                                                 // update and draw the run
    void updateInput(InputEvent *event);                                                             // update input for the run
};
