#include "run/run.hpp"
#include "app.hpp"

FlipSocialRun::FlipSocialRun(void *appContext) : appContext(appContext),
                                                 currentMenuIndex(SocialViewFeed), currentProfileElement(ProfileElementBio), currentView(SocialViewLogin),
                                                 inputHeld(false), lastInput(InputKeyMAX),
                                                 loginStatus(LoginNotStarted), registrationStatus(RegistrationNotStarted),
                                                 shouldDebounce(false), shouldReturnToMenu(false), userInfoStatus(UserInfoNotStarted)
{
    char *loginStatusStr = (char *)malloc(64);
    if (loginStatusStr)
    {
        FlipSocialApp *app = static_cast<FlipSocialApp *>(appContext);
        if (app && app->loadChar("login_status", loginStatusStr, 64))
        {
            if (strcmp(loginStatusStr, "success") == 0)
            {
                loginStatus = LoginSuccess;
                currentView = SocialViewMenu;
            }
            else
            {
                loginStatus = LoginNotStarted;
                currentView = SocialViewLogin;
            }
        }
        free(loginStatusStr);
    }
}

FlipSocialRun::~FlipSocialRun()
{
    // nothing to do
}

void FlipSocialRun::debounceInput()
{
    static uint8_t debounceCounter = 0;
    if (shouldDebounce)
    {
        lastInput = InputKeyMAX;
        debounceCounter++;
        if (debounceCounter < 2)
        {
            return;
        }
        debounceCounter = 0;
        shouldDebounce = false;
        inputHeld = false;
    }
}

void FlipSocialRun::drawLoginView(Canvas *canvas)
{
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    static bool loadingStarted = false;
    switch (loginStatus)
    {
    case LoginWaiting:
        if (!loadingStarted)
        {
            if (!loading)
            {
                loading = std::make_unique<Loading>(canvas);
            }
            loadingStarted = true;
            if (loading)
            {
                loading->setText("Logging in...");
            }
        }
        if (!this->httpRequestIsFinished())
        {
            if (loading)
            {
                loading->animate();
            }
        }
        else
        {
            if (loading)
            {
                loading->stop();
            }
            loadingStarted = false;
            FlipSocialApp *app = static_cast<FlipSocialApp *>(appContext);
            if (app->getHttpState() == ISSUE)
            {
                loginStatus = LoginRequestError;
                return;
            }
            char response[256];
            if (app && app->loadChar("login", response, sizeof(response)))
            {
                if (strstr(response, "[SUCCESS]") != NULL)
                {
                    loginStatus = LoginSuccess;
                    currentView = SocialViewMenu;
                }
                else if (strstr(response, "User not found") != NULL)
                {
                    loginStatus = LoginNotStarted;
                    currentView = SocialViewRegistration;
                    registrationStatus = RegistrationWaiting;
                    userRequest(RequestTypeRegistration);
                }
                else if (strstr(response, "Incorrect password") != NULL)
                {
                    loginStatus = LoginWrongPassword;
                }
                else if (strstr(response, "Username or password is empty.") != NULL)
                {
                    loginStatus = LoginCredentialsMissing;
                }
                else
                {
                    loginStatus = LoginRequestError;
                }
            }
            else
            {
                loginStatus = LoginRequestError;
            }
        }
        break;
    case LoginSuccess:
        canvas_draw_str(canvas, 0, 10, "Login successful!");
        canvas_draw_str(canvas, 0, 20, "Press OK to continue.");
        break;
    case LoginCredentialsMissing:
        canvas_draw_str(canvas, 0, 10, "Missing credentials!");
        canvas_draw_str(canvas, 0, 20, "Please set your username");
        canvas_draw_str(canvas, 0, 30, "and password in the app.");
        break;
    case LoginRequestError:
        canvas_draw_str(canvas, 0, 10, "Login request failed!");
        canvas_draw_str(canvas, 0, 20, "Check your network and");
        canvas_draw_str(canvas, 0, 30, "try again later.");
        break;
    case LoginWrongPassword:
        canvas_draw_str(canvas, 0, 10, "Wrong password!");
        canvas_draw_str(canvas, 0, 20, "Please check your password");
        canvas_draw_str(canvas, 0, 30, "and try again.");
        break;
    case LoginNotStarted:
        loginStatus = LoginWaiting;
        userRequest(RequestTypeLogin);
        break;
    default:
        canvas_draw_str(canvas, 0, 10, "Logging in...");
        break;
    }
}

void FlipSocialRun::drawMainMenuView(Canvas *canvas)
{
    const char *menuItems[] = {"Feed", "Messages", "Profile"};
    drawMenu(canvas, (uint8_t)currentMenuIndex, menuItems, 3);
}

void FlipSocialRun::drawMenu(Canvas *canvas, uint8_t selectedIndex, const char **menuItems, uint8_t menuCount)
{
    canvas_clear(canvas);

    // Draw title
    canvas_set_font_custom(canvas, FONT_SIZE_LARGE);
    const char *title = "FlipSocial";
    int title_width = canvas_string_width(canvas, title);
    int title_x = (128 - title_width) / 2; // Center the title
    canvas_draw_str(canvas, title_x, 12, title);

    // Draw underline for title
    canvas_draw_line(canvas, title_x, 14, title_x + title_width, 14);

    // Draw decorative horizontal pattern
    for (int i = 0; i < 128; i += 4)
    {
        canvas_draw_dot(canvas, i, 18);
    }

    // Menu items - horizontal scrolling, show one at a time
    canvas_set_font_custom(canvas, FONT_SIZE_MEDIUM);

    // Display current menu item centered
    const char *currentItem = menuItems[selectedIndex];
    int text_width = canvas_string_width(canvas, currentItem);
    int text_x = (128 - text_width) / 2;
    int menu_y = 40; // Centered vertically

    // Draw main selection box for current item
    canvas_draw_rbox(canvas, 10, menu_y - 8, 108, 16, 4);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_str(canvas, text_x, menu_y + 4, currentItem);
    canvas_set_color(canvas, ColorBlack);

    // Draw navigation arrows
    if (selectedIndex > 0)
    {
        canvas_draw_str(canvas, 2, menu_y + 4, "<");
    }
    if (selectedIndex < (menuCount - 1))
    {
        canvas_draw_str(canvas, 122, menu_y + 4, ">");
    }

    // Draw page indicator dots
    int dots_spacing = 6;
    int dots_start_x = (128 - (menuCount * dots_spacing)) / 2; // Center dots with spacing
    for (int i = 0; i < menuCount; i++)
    {
        int dot_x = dots_start_x + (i * dots_spacing);
        if (i == selectedIndex)
        {
            canvas_draw_box(canvas, dot_x, menu_y + 12, 4, 4); // Filled dot for current
        }
        else
        {
            canvas_draw_frame(canvas, dot_x, menu_y + 12, 4, 4); // Empty dot for others
        }
    }

    // Draw decorative bottom pattern
    for (int i = 0; i < 128; i += 4)
    {
        canvas_draw_dot(canvas, i, 58);
    }
}

void FlipSocialRun::drawProfileView(Canvas *canvas)
{
    canvas_clear(canvas);

    FlipSocialApp *app = static_cast<FlipSocialApp *>(appContext);
    if (!app)
    {
        FURI_LOG_E(TAG, "drawProfileView: App context is null");
        return;
    }

    char userInfo[256];
    if (!app->loadChar("user_info", userInfo, sizeof(userInfo)))
    {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter,
                                "Failed to load user info.");
        return;
    }

    char username[64];
    if (!app->loadChar("user_name", username, sizeof(username)))
    {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter,
                                "Failed to load username.");
        return;
    }

    char *bio = get_json_value("bio", userInfo);
    char *friendsCount = get_json_value("friends_count", userInfo);
    char *dateCreated = get_json_value("date_created", userInfo);

    if (!bio || !friendsCount || !dateCreated)
    {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter,
                                "Incomplete profile data.");
        if (bio)
            free(bio);
        if (friendsCount)
            free(friendsCount);
        if (dateCreated)
            free(dateCreated);
        return;
    }

    canvas_set_font_custom(canvas, FONT_SIZE_LARGE);
    int title_width = canvas_string_width(canvas, username);
    int title_x = (128 - title_width) / 2;
    canvas_draw_str(canvas, title_x, 12, username);

    // underline for username
    canvas_draw_line(canvas, title_x, 14, title_x + title_width, 14);

    // decorative top pattern
    for (int i = 0; i < 128; i += 4)
    {
        canvas_draw_dot(canvas, i, 18);
    }

    // Profile element labels
    const char *elementLabels[] = {"Bio", "Friends", "Joined"};

    // current element label
    canvas_set_font_custom(canvas, FONT_SIZE_MEDIUM);
    const char *currentLabel = elementLabels[currentProfileElement];
    int label_width = canvas_string_width(canvas, currentLabel);
    int label_x = (128 - label_width) / 2;
    canvas_draw_str(canvas, label_x, 28, currentLabel);

    // main content box
    canvas_draw_rbox(canvas, 7, 32, 114, 20, 4);
    canvas_set_color(canvas, ColorWhite);

    // Draw content based on current element
    canvas_set_font_custom(canvas, FONT_SIZE_SMALL);
    switch (currentProfileElement)
    {
    case ProfileElementBio:
        if (strlen(bio) == 0)
        {
            canvas_draw_str_aligned(canvas, 64, 42, AlignCenter, AlignCenter, "No bio");
        }
        else
        {
            drawWrappedBio(canvas, bio, 10, 40);
        }
        break;
    case ProfileElementFriends:
    {
        canvas_draw_str_aligned(canvas, 64, 42, AlignCenter, AlignCenter, friendsCount);
    }
    break;
    case ProfileElementJoined:
    {
        canvas_draw_str_aligned(canvas, 64, 42, AlignCenter, AlignCenter, dateCreated);
    }
    break;
    default:
        canvas_draw_str_aligned(canvas, 64, 42, AlignCenter, AlignCenter, "Unknown");
        break;
    }

    canvas_set_color(canvas, ColorBlack);

    // navigation arrows
    if (currentProfileElement > 0)
    {
        canvas_draw_str(canvas, 2, 42, "<");
    }
    if (currentProfileElement < (ProfileElementMAX - 1))
    {
        canvas_draw_str(canvas, 122, 42, ">");
    }

    // page indicator dots
    int dots_spacing = 6;
    int dots_start_x = (128 - (ProfileElementMAX * dots_spacing)) / 2;
    for (int i = 0; i < ProfileElementMAX; i++)
    {
        int dot_x = dots_start_x + (i * dots_spacing);
        if (i == currentProfileElement)
        {
            canvas_draw_box(canvas, dot_x, 56, 4, 4);
        }
        else
        {
            canvas_draw_frame(canvas, dot_x, 56, 4, 4);
        }
    }

    // decorative bottom pattern
    for (int i = 0; i < 128; i += 4)
    {
        canvas_draw_dot(canvas, i, 62);
    }

    free(bio);
    free(friendsCount);
    free(dateCreated);
}

void FlipSocialRun::drawWrappedBio(Canvas *canvas, const char *text, uint8_t x, uint8_t y)
{
    if (!text || strlen(text) == 0)
    {
        canvas_draw_str_aligned(canvas, 64, y + 2, AlignCenter, AlignCenter, "No bio");
        return;
    }

    const uint8_t maxCharsPerLine = 18;
    uint8_t textLen = strlen(text);

    if (textLen <= maxCharsPerLine)
    {
        canvas_draw_str_aligned(canvas, 64, y + 2, AlignCenter, AlignCenter, text);
        return;
    }

    char line1[maxCharsPerLine + 1] = {0};
    char line2[maxCharsPerLine + 1] = {0};

    // First line
    uint8_t line1Len = (textLen > maxCharsPerLine) ? maxCharsPerLine : textLen;

    // Try to break at word boundary for first line
    uint8_t breakPoint = line1Len;
    if (textLen > maxCharsPerLine)
    {
        for (int i = maxCharsPerLine - 1; i > maxCharsPerLine - 8; i--) // Look back up to 7 chars
        {
            if (text[i] == ' ')
            {
                breakPoint = i;
                break;
            }
        }
    }

    strncpy(line1, text, breakPoint);
    line1[breakPoint] = '\0';

    // Second line
    if (textLen > breakPoint)
    {
        uint8_t remainingLen = textLen - breakPoint;
        if (text[breakPoint] == ' ')
            breakPoint++; // Skip space at beginning of line2

        uint8_t line2Len = (remainingLen > maxCharsPerLine) ? maxCharsPerLine - 3 : remainingLen;
        strncpy(line2, &text[breakPoint], line2Len);

        // Add ellipsis if text is truncated
        if (remainingLen > maxCharsPerLine)
        {
            line2[line2Len - 3] = '.';
            line2[line2Len - 2] = '.';
            line2[line2Len - 1] = '.';
        }
        line2[line2Len] = '\0';
    }

    // Draw the lines
    canvas_draw_str(canvas, x, y, line1);
    if (strlen(line2) > 0)
    {
        canvas_draw_str(canvas, x, y + 8, line2);
    }
}

void FlipSocialRun::drawRegistrationView(Canvas *canvas)
{
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    static bool loadingStarted = false;
    switch (registrationStatus)
    {
    case RegistrationWaiting:
        if (!loadingStarted)
        {
            if (!loading)
            {
                loading = std::make_unique<Loading>(canvas);
            }
            loadingStarted = true;
            if (loading)
            {
                loading->setText("Registering...");
            }
        }
        if (!this->httpRequestIsFinished())
        {
            if (loading)
            {
                loading->animate();
            }
        }
        else
        {
            if (loading)
            {
                loading->stop();
            }
            loadingStarted = false;
            FlipSocialApp *app = static_cast<FlipSocialApp *>(appContext);
            if (app->getHttpState() == ISSUE)
            {
                registrationStatus = RegistrationRequestError;
                return;
            }
            char response[256];
            if (app && app->loadChar("register", response, sizeof(response)))
            {
                if (strstr(response, "[SUCCESS]") != NULL)
                {
                    registrationStatus = RegistrationSuccess;
                    currentView = SocialViewMenu;
                }
                else if (strstr(response, "Username or password not provided") != NULL)
                {
                    registrationStatus = RegistrationCredentialsMissing;
                }
                else if (strstr(response, "User already exists") != NULL)
                {
                    registrationStatus = RegistrationUserExists;
                }
                else
                {
                    registrationStatus = RegistrationRequestError;
                }
            }
            else
            {
                registrationStatus = RegistrationRequestError;
            }
        }
        break;
    case RegistrationSuccess:
        canvas_draw_str(canvas, 0, 10, "Registration successful!");
        canvas_draw_str(canvas, 0, 20, "Press OK to continue.");
        break;
    case RegistrationCredentialsMissing:
        canvas_draw_str(canvas, 0, 10, "Missing credentials!");
        canvas_draw_str(canvas, 0, 20, "Please set your username");
        canvas_draw_str(canvas, 0, 30, "and password in the app.");
        break;
    case RegistrationRequestError:
        canvas_draw_str(canvas, 0, 10, "Registration request failed!");
        canvas_draw_str(canvas, 0, 20, "Check your network and");
        canvas_draw_str(canvas, 0, 30, "try again later.");
        break;
    default:
        canvas_draw_str(canvas, 0, 10, "Registering...");
        break;
    }
}

void FlipSocialRun::drawUserInfoView(Canvas *canvas)
{
    static bool loadingStarted = false;
    switch (userInfoStatus)
    {
    case UserInfoWaiting:
        if (!loadingStarted)
        {
            if (!loading)
            {
                loading = std::make_unique<Loading>(canvas);
            }
            loadingStarted = true;
            if (loading)
            {
                loading->setText("Syncing...");
            }
        }
        if (!this->httpRequestIsFinished())
        {
            if (loading)
            {
                loading->animate();
            }
        }
        else
        {
            canvas_draw_str(canvas, 0, 10, "Loading user info...");
            canvas_draw_str(canvas, 0, 20, "Please wait...");
            canvas_draw_str(canvas, 0, 30, "It may take up to 15 seconds.");
            FlipSocialApp *app = static_cast<FlipSocialApp *>(appContext);
            if (app->getHttpState() == ISSUE)
            {
                userInfoStatus = UserInfoRequestError;
                if (loading)
                {
                    loading->stop();
                }
                loadingStarted = false;
                return;
            }
            char response[512];
            if (app && app->loadChar("user_info", response, sizeof(response)))
            {
                userInfoStatus = UserInfoSuccess;
                app->saveChar("login_status", "success");
                if (loading)
                {
                    loading->stop();
                }
                loadingStarted = false;
                currentView = SocialViewProfile;
                return;
            }
            else
            {
                userInfoStatus = UserInfoRequestError;
            }
        }
        break;
    case UserInfoSuccess:
        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 0, 10, "User info loaded successfully!");
        canvas_draw_str(canvas, 0, 20, "Press OK to continue.");
        break;
    case UserInfoCredentialsMissing:
        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 0, 10, "Missing credentials!");
        canvas_draw_str(canvas, 0, 20, "Please update your username");
        canvas_draw_str(canvas, 0, 30, "and password in the settings.");
        break;
    case UserInfoRequestError:
        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 0, 10, "User info request failed!");
        canvas_draw_str(canvas, 0, 20, "Check your network and");
        canvas_draw_str(canvas, 0, 30, "try again later.");
        break;
    case UserInfoParseError:
        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 0, 10, "Failed to parse user info!");
        canvas_draw_str(canvas, 0, 20, "Try again...");
        break;
    default:
        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 0, 10, "Loading user info...");
        break;
    }
}

bool FlipSocialRun::httpRequestIsFinished()
{
    FlipSocialApp *app = static_cast<FlipSocialApp *>(appContext);
    if (!app)
    {
        return true;
    }
    auto state = app->getHttpState();
    return state == IDLE || state == ISSUE || state == INACTIVE;
}

void FlipSocialRun::updateDraw(Canvas *canvas)
{
    canvas_clear(canvas);
    switch (currentView)
    {
    case SocialViewMenu:
        drawMainMenuView(canvas);
        break;
    case SocialViewLogin:
        drawLoginView(canvas);
        break;
    case SocialViewRegistration:
        drawRegistrationView(canvas);
        break;
    case SocialViewUserInfo:
        drawUserInfoView(canvas);
        break;
    case SocialViewProfile:
        drawProfileView(canvas);
        break;
    default:
        canvas_draw_str(canvas, 0, 10, "View not implemented yet.");
        break;
    };
}

void FlipSocialRun::updateInput(InputEvent *event)
{
    lastInput = event->key;
    debounceInput();
    switch (currentView)
    {
    case SocialViewMenu:
    {
        switch (lastInput)
        {
        case InputKeyBack:
            // return to menu
            shouldReturnToMenu = true;
            break;
        case InputKeyDown:
        case InputKeyLeft:
            if (currentMenuIndex == SocialViewMessages)
            {
                currentMenuIndex = SocialViewFeed;
            }
            else if (currentMenuIndex == SocialViewProfile)
            {
                currentMenuIndex = SocialViewMessages;
                shouldDebounce = true;
            }
            break;
        case InputKeyUp:
        case InputKeyRight:
            if (currentMenuIndex == SocialViewFeed)
            {
                currentMenuIndex = SocialViewMessages;
                shouldDebounce = true;
            }
            else if (currentMenuIndex == SocialViewMessages)
            {
                currentMenuIndex = SocialViewProfile;
            }
            break;
        case InputKeyOk:
            switch (currentMenuIndex)
            {
            case SocialViewFeed:
                currentView = SocialViewFeed;
                shouldDebounce = true;
                break;
            case SocialViewMessages:
                currentView = SocialViewMessages;
                shouldDebounce = true;
                break;
            case SocialViewProfile:
                if (userInfoStatus == UserInfoNotStarted || userInfoStatus == UserInfoRequestError)
                {
                    currentView = SocialViewUserInfo;
                    userInfoStatus = UserInfoWaiting;
                    userRequest(RequestTypeUserInfo);
                }
                else if (userInfoStatus == UserInfoSuccess)
                {
                    currentView = SocialViewProfile;
                }
                shouldDebounce = true;
                break;
            default:
                break;
            };
            break;
        default:
            break;
        }
        break;
    }
    case SocialViewFeed:
    {
        switch (lastInput)
        {
        case InputKeyBack:
            currentView = SocialViewMenu;
            shouldDebounce = true;
            break;
        default:
            break;
        };
        break;
    }
    case SocialViewMessages:
    {
        switch (lastInput)
        {
        case InputKeyBack:
            currentView = SocialViewMenu;
            shouldDebounce = true;
            break;
        default:
            break;
        };
        break;
    }
    case SocialViewProfile:
    {
        switch (lastInput)
        {
        case InputKeyBack:
            currentView = SocialViewMenu;
            shouldDebounce = true;
            break;
        case InputKeyLeft:
        case InputKeyDown:
            if (currentProfileElement > 0)
            {
                currentProfileElement--;
                shouldDebounce = true;
            }
            break;
        case InputKeyRight:
        case InputKeyUp:
            if (currentProfileElement < (ProfileElementMAX - 1))
            {
                currentProfileElement++;
                shouldDebounce = true;
            }
            break;
        default:
            break;
        };
        break;
    }
    case SocialViewLogin:
    case SocialViewRegistration:
    case SocialViewUserInfo:
    {
        if (lastInput == InputKeyBack)
        {
            currentView = SocialViewLogin;
            shouldReturnToMenu = true;
            shouldDebounce = true;
        }
        break;
    }
    default:
        break;
    };
}

void FlipSocialRun::userRequest(RequestType requestType)
{
    // Get app context to access HTTP functionality
    FlipSocialApp *app = static_cast<FlipSocialApp *>(appContext);
    if (!app)
    {
        FURI_LOG_E(TAG, "userRequest: App context is null");
        return;
    }

    // Allocate memory for credentials
    char *username = (char *)malloc(64);
    char *password = (char *)malloc(64);
    if (!username || !password)
    {
        FURI_LOG_E(TAG, "userRequest: Failed to allocate memory for credentials");
        if (username)
            free(username);
        if (password)
            free(password);
        return;
    }

    // Load credentials from storage
    bool credentialsLoaded = true;
    if (!app->loadChar("user_name", username, 64))
    {
        FURI_LOG_E(TAG, "Failed to load user_name");
        credentialsLoaded = false;
    }
    if (!app->loadChar("user_pass", password, 64))
    {
        FURI_LOG_E(TAG, "Failed to load user_pass");
        credentialsLoaded = false;
    }

    if (!credentialsLoaded)
    {
        switch (requestType)
        {
        case RequestTypeLogin:
            loginStatus = LoginCredentialsMissing;
            break;
        case RequestTypeRegistration:
            registrationStatus = RegistrationCredentialsMissing;
            break;
        case RequestTypeUserInfo:
            userInfoStatus = UserInfoCredentialsMissing;
            break;
        default:
            FURI_LOG_E(TAG, "Unknown request type: %d", requestType);
            loginStatus = LoginRequestError;
            registrationStatus = RegistrationRequestError;
            userInfoStatus = UserInfoRequestError;
            break;
        }
        free(username);
        free(password);
        return;
    }

    // Create JSON payload for login/registration
    char *payload = (char *)malloc(256);
    if (!payload)
    {
        FURI_LOG_E(TAG, "userRequest: Failed to allocate memory for payload");
        free(username);
        free(password);
        return;
    }
    snprintf(payload, 256, "{\"username\":\"%s\",\"password\":\"%s\"}", username, password);

    switch (requestType)
    {
    case RequestTypeLogin:
        if (!app->httpRequestAsync("login.txt",
                                   "https://www.jblanked.com/flipper/api/user/login/",
                                   POST, "{\"Content-Type\":\"application/json\"}", payload))
        {
            loginStatus = LoginRequestError;
        }
        break;
    case RequestTypeRegistration:
        if (!app->httpRequestAsync("register.txt",
                                   "https://www.jblanked.com/flipper/api/user/register/",
                                   POST, "{\"Content-Type\":\"application/json\"}", payload))
        {
            registrationStatus = RegistrationRequestError;
        }
        break;
    case RequestTypeUserInfo:
    {
        char *url = (char *)malloc(128);
        if (!url)
        {
            FURI_LOG_E(TAG, "userRequest: Failed to allocate memory for url");
            userInfoStatus = UserInfoRequestError;
            free(username);
            free(password);
            free(payload);
            return;
        }
        snprintf(url, 128, "https://www.jblanked.com/flipper/api/user/profile/%s/", username);
        if (!app->httpRequestAsync("user_info.txt", url, GET, "{\"Content-Type\":\"application/json\"}"))
        {
            userInfoStatus = UserInfoRequestError;
        }
        free(url);
        break;
    }
    default:
        FURI_LOG_E(TAG, "Unknown request type: %d", requestType);
        loginStatus = LoginRequestError;
        registrationStatus = RegistrationRequestError;
        userInfoStatus = UserInfoRequestError;
        free(username);
        free(password);
        free(payload);
        return;
    }

    free(username);
    free(password);
    free(payload);
}