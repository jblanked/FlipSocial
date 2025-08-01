#include "run/run.hpp"
#include "app.hpp"
#include <vector>

FlipSocialRun::FlipSocialRun(void *appContext) : appContext(appContext),
                                                 currentMenuIndex(SocialViewFeed), currentProfileElement(ProfileElementBio), currentView(SocialViewLogin),
                                                 feedItemID(0), feedItemIndex(0), feedIteration(1), feedStatus(FeedNotStarted), inputHeld(false), lastInput(InputKeyMAX),
                                                 loginStatus(LoginNotStarted), messageUsersStatus(MessageUsersNotStarted), messageUserIndex(0), registrationStatus(RegistrationNotStarted),
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

void FlipSocialRun::drawFeedItem(Canvas *canvas, char *username, char *message, char *flipped, char *flips, char *date_created)
{
    bool isFlipped = strcmp(flipped, "true") == 0;
    auto flipCount = atoi(flips);
    canvas_clear(canvas);
    canvas_set_font_custom(canvas, FONT_SIZE_LARGE);
    canvas_draw_str(canvas, 0, 7, username);
    canvas_set_font_custom(canvas, FONT_SIZE_MEDIUM);
    drawFeedMessage(canvas, message, 0, 12);
    canvas_set_font_custom(canvas, FONT_SIZE_SMALL);
    char flip_message[32];
    snprintf(flip_message, sizeof(flip_message), "%u %s", flipCount, flipCount == 1 ? "flip" : "flips");
    canvas_draw_str(canvas, 0, 60, flip_message);
    char flip_status[16];
    snprintf(flip_status, sizeof(flip_status), isFlipped ? "Unflip" : "Flip");
    canvas_draw_str(canvas, 32, 60, flip_status);
    canvas_draw_str(canvas, 64, 60, date_created);
}

void FlipSocialRun::drawFeedMessage(Canvas *canvas, const char *user_message, int x, int y)
{
    if (!user_message)
    {
        FURI_LOG_E(TAG, "User message is NULL.");
        return;
    }

    // We will read through user_message and extract words manually
    const char *p = user_message;

    // Skip leading spaces
    while (*p == ' ')
        p++;

    char line[128];
    size_t line_len = 0;
    line[0] = '\0';
    int line_num = 0;

    while (*p && line_num < 6)
    {
        // Find the end of the next word
        const char *word_start = p;
        while (*p && *p != ' ')
            p++;
        size_t word_len = p - word_start;

        // Extract the word into a temporary buffer
        char word[128];
        if (word_len > 127)
        {
            word_len = 127; // Just to avoid overflow if extremely large
        }
        memcpy(word, word_start, word_len);
        word[word_len] = '\0';

        // Skip trailing spaces for the next iteration
        while (*p == ' ')
            p++;

        if (word_len == 0)
        {
            // Empty word (consecutive spaces?), just continue
            continue;
        }

        // Check how the word fits into the current line
        char test_line[256];
        if (line_len == 0)
        {
            // If line is empty, the line would just be this word
            strncpy(test_line, word, sizeof(test_line) - 1);
            test_line[sizeof(test_line) - 1] = '\0';
        }
        else
        {
            // If not empty, we add a space and then the word
            snprintf(test_line, sizeof(test_line), "%s %s", line, word);
        }

        uint16_t width = canvas_string_width(canvas, test_line);
        if (width <= 128)
        {
            // The word fits on this line
            strcpy(line, test_line);
            line_len = strlen(line);
        }
        else
        {
            // The word doesn't fit on this line
            // First, draw the current line if it's not empty
            if (line_len > 0)
            {
                canvas_draw_str_aligned(canvas, x, y + line_num * 8, AlignLeft, AlignTop, line);
                line_num++;
                if (line_num >= 6)
                    break;
            }

            // Now we try to put the current word on a new line
            // Check if the word itself fits on an empty line
            width = canvas_string_width(canvas, word);
            if (width <= 128)
            {
                // The whole word fits on a new line
                strcpy(line, word);
                line_len = word_len;
            }
            else
            {
                // The word alone doesn't fit. We must truncate it.
                // We'll find the largest substring of the word that fits.
                size_t truncate_len = word_len;
                while (truncate_len > 0)
                {
                    char truncated[128];
                    strncpy(truncated, word, truncate_len);
                    truncated[truncate_len] = '\0';
                    if (canvas_string_width(canvas, truncated) <= 128)
                    {
                        // Found a substring that fits
                        strcpy(line, truncated);
                        line_len = truncate_len;
                        break;
                    }
                    truncate_len--;
                }

                if (line_len == 0)
                {
                    // Could not fit a single character. Skip this word.
                }
            }
        }
    }

    // Draw any remaining text in the buffer if we have lines left
    if (line_len > 0 && line_num < 6)
    {
        canvas_draw_str_aligned(canvas, x, y + line_num * 8, AlignLeft, AlignTop, line);
    }
}

void FlipSocialRun::drawFeedView(Canvas *canvas)
{
    static bool loadingStarted = false;
    switch (feedStatus)
    {
    case FeedWaiting:
        if (!loadingStarted)
        {
            if (!loading)
            {
                loading = std::make_unique<Loading>(canvas);
            }
            loadingStarted = true;
            if (loading)
            {
                loading->setText("Fetching...");
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
            FlipSocialApp *app = static_cast<FlipSocialApp *>(appContext);
            if (app && app->getHttpState() == ISSUE)
            {
                feedStatus = FeedRequestError;
                if (loading)
                {
                    loading->stop();
                }
                loadingStarted = false;
                return;
            }
            char *response = (char *)malloc(4096);
            char *feedSaveKey = (char *)malloc(16);
            if (!response || !feedSaveKey)
            {
                feedStatus = FeedParseError;
                if (response)
                    free(response);
                if (feedSaveKey)
                    free(feedSaveKey);
                if (loading)
                {
                    loading->stop();
                }
                loadingStarted = false;
                return;
            }
            snprintf(feedSaveKey, 16, "feed_%d", feedIteration);
            if (app && app->loadChar(feedSaveKey, response, 4096))
            {
                feedStatus = FeedSuccess;
                if (loading)
                {
                    loading->stop();
                }
                loadingStarted = false;
                free(response);
                free(feedSaveKey);
                return;
            }
            feedStatus = FeedRequestError;
            free(response);
            free(feedSaveKey);
        }
        break;
    case FeedSuccess:
    {
        /* example response
        {"feed":[{"username":"Marc0132","message":"updated second flipper =-)","flipped":false,"id":5446,"flip_count":0,"date_created":"1 hours ago"},{"username":"Marc013","message":"helping others its a great thing. maybe the best of thing=-) ","flipped":false,"id":5445,"flip_count":1,"date_created":"1 hours ago"},{"username":"KooTeR","message":"KJB Flipper Online 1st time","flipped":false,"id":5444,"flip_count":3,"date_created":"1 hours ago"},{"username":"belze743","message":"Scytale5;nc olieto","flipped":false,"id":5443,"flip_count":1,"date_created":"5 hours ago"},{"username":"Marc013","message":"i hope you all have a great hacking life i do LoL =-)","flipped":false,"id":5442,"flip_count":1,"date_created":"10 hours ago"},{"username":"JBlanked","message":"Morning all :D","flipped":false,"id":5441,"flip_count":1,"date_created":"11 hours ago"},{"username":"Marc013","message":"GREAT DAY TO ALL","flipped":false,"id":5440,"flip_count":0,"date_created":"14 hours ago"},{"username":"Turtle","message":"Cheese","flipped":false,"id":5439,"flip_count":1,"date_created":"18 hours ago"},{"username":"ShadowScout101","message":"Goodnight everyone!","flipped":false,"id":5438,"flip_count":1,"date_created":"22 hours ago"},{"username":"ShadowScout101","message":"@WildWill Customize your shortpresses and holds in the MNTM settings to make life easy.","flipped":false,"id":5437,"flip_count":1,"date_created":"Yesterday"},{"username":"Marc013","message":"my flipper have all 270apps and level29 9909/9999 =-) i like this world ","flipped":false,"id":5436,"flip_count":2,"date_created":"Yesterday"},{"username":"Marc013","message":"Great nigth to all =-)","flipped":false,"id":5435,"flip_count":3,"date_created":"Yesterday"}]}
        */
        char *response = (char *)malloc(4096);
        char *feedSaveKey = (char *)malloc(16);
        if (!response || !feedSaveKey)
        {
            feedStatus = FeedParseError;
            if (response)
            {
                free(response);
            }
            if (feedSaveKey)
            {
                free(feedSaveKey);
            }
            canvas_draw_str(canvas, 0, 10, "Failed to load feed data.");
            return;
        }
        snprintf(feedSaveKey, 16, "feed_%d", feedIteration);
        FlipSocialApp *app = static_cast<FlipSocialApp *>(appContext);
        if (app && app->loadChar(feedSaveKey, response, 4096))
        {
            // Successfully loaded feed data
            // Parse and display feed data...
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "Feed Items:");

            for (int i = 0; i < MAX_FEED_ITEMS; i++)
            {
                if (i != feedItemIndex)
                {
                    continue;
                }
                // only draw the current displayed feed item
                char *feedItem = get_json_array_value("feed", i, response);
                if (feedItem)
                {
                    char *username = get_json_value("username", feedItem);
                    char *message = get_json_value("message", feedItem);
                    char *flipped = get_json_value("flipped", feedItem);
                    char *flips_str = get_json_value("flip_count", feedItem);
                    char *date_created = get_json_value("date_created", feedItem);
                    char *id_str = get_json_value("id", feedItem);
                    if (!username || !message || !flipped || !flips_str || !date_created || !id_str)
                    {
                        if (username)
                            free(username);
                        if (message)
                            free(message);
                        if (flipped)
                            free(flipped);
                        if (flips_str)
                            free(flips_str);
                        if (date_created)
                            free(date_created);
                        if (id_str)
                            free(id_str);
                        free(feedItem);
                        return;
                    }
                    feedItemID = atoi(id_str);
                    drawFeedItem(canvas, username, message, flipped, flips_str, date_created);
                    //
                    free(username);
                    free(message);
                    free(flipped);
                    free(flips_str);
                    free(date_created);
                    free(id_str);
                    free(feedItem);
                }
            }
        }
        free(response);
        free(feedSaveKey);
        break;
    }
    case FeedRequestError:
        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 0, 10, "Feed request failed!");
        canvas_draw_str(canvas, 0, 20, "Check your network and");
        canvas_draw_str(canvas, 0, 30, "try again later.");
        break;
    case FeedParseError:
        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 0, 10, "Failed to parse feed!");
        canvas_draw_str(canvas, 0, 20, "Try again...");
        break;
    case FeedNotStarted:
        canvas_clear(canvas);
        feedStatus = FeedWaiting;
        userRequest(RequestTypeFeed);
        break;
    default:
        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 0, 10, "Loading feed...");
        break;
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

void FlipSocialRun::drawMessageUsersView(Canvas *canvas)
{
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    static bool loadingStarted = false;
    switch (messageUsersStatus)
    {
    case MessageUsersWaiting:
        if (!loadingStarted)
        {
            if (!loading)
            {
                loading = std::make_unique<Loading>(canvas);
            }
            loadingStarted = true;
            if (loading)
            {
                loading->setText("Retrieving...");
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
                messageUsersStatus = MessageUsersRequestError;
                return;
            }
            char *response = (char *)malloc(1024);
            if (response && app->loadChar("messages_user_list", response, 1024) && strstr(response, "users") != NULL)
            {
                messageUsersStatus = MessageUsersSuccess;
                free(response);
                return;
            }
            else
            {
                messageUsersStatus = MessageUsersRequestError;
            }
        }
        break;
    case MessageUsersSuccess:
    {
        canvas_draw_str(canvas, 0, 10, "Messages retrieved successfully!");
        canvas_draw_str(canvas, 0, 20, "Press OK to continue.");
        char *messagesUserList = (char *)malloc(1024);
        if (!messagesUserList)
        {
            FURI_LOG_E(TAG, "drawMessageUsersView: Failed to allocate memory for messagesUserList");
            messageUsersStatus = MessageUsersParseError;
            return;
        }

        FlipSocialApp *app = static_cast<FlipSocialApp *>(appContext);
        if (!app || !app->loadChar("messages_user_list", messagesUserList, 1024))
        {
            FURI_LOG_E(TAG, "drawMessageUsersView: Failed to load messages user list from storage");
            canvas_draw_str(canvas, 0, 30, "Failed to load messages.");
            free(messagesUserList);
            return;
        }

        // store users
        std::vector<std::string> usersList;
        for (int i = 0; i < MAX_MESSAGE_USERS; i++)
        {
            char *user = get_json_array_value("users", i, messagesUserList);
            if (!user)
            {
                break; // No more users in the list
            }
            usersList.push_back(user);
            free(user);
        }

        if (usersList.empty())
        {
            canvas_draw_str(canvas, 0, 30, "No messages found.");
        }
        else
        {
            // std::vector<std::string> to const char** for drawMenu
            std::vector<const char *> userPtrs;
            userPtrs.reserve(usersList.size());
            for (const auto &user : usersList)
            {
                userPtrs.push_back(user.c_str());
            }
            drawMenu(canvas, messageUserIndex, userPtrs.data(), userPtrs.size());
        }
        free(messagesUserList);
        break;
    }
    case MessageUsersRequestError:
        canvas_draw_str(canvas, 0, 10, "Messages request failed!");
        canvas_draw_str(canvas, 0, 20, "Check your network and");
        canvas_draw_str(canvas, 0, 30, "try again later.");
        break;
    case MessageUsersParseError:
        canvas_draw_str(canvas, 0, 10, "Error parsing messages!");
        canvas_draw_str(canvas, 0, 20, "Please set your username");
        canvas_draw_str(canvas, 0, 30, "and password in the app.");
        break;
    case MessageUsersNotStarted:
        messageUsersStatus = MessageUsersWaiting;
        userRequest(RequestTypeMessagesUserList);
        break;
    default:
        canvas_draw_str(canvas, 0, 10, "Retrieving messages...");
        break;
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

bool FlipSocialRun::getMessageUser(uint8_t index, char *buffer, size_t buffer_size)
{
    char *messagesUserList = (char *)malloc(1024);
    FlipSocialApp *app = static_cast<FlipSocialApp *>(appContext);
    if (!app || !messagesUserList || !app->loadChar("messages_user_list", messagesUserList, 1024))
    {
        FURI_LOG_E(TAG, "drawMessageUsersView: Failed to load messages user list from storage");
        free(messagesUserList);
        return false;
    }
    for (int i = 0; i < MAX_MESSAGE_USERS; i++)
    {
        if (i != index)
        {
            continue; // Only return the requested user
        }
        char *user = get_json_array_value("users", i, messagesUserList);
        if (user && strlen(user) > 0)
        {
            snprintf(buffer, buffer_size, "%s", user);
            free(user);
            free(messagesUserList);
            return true;
        }
    }
    free(messagesUserList);
    return false;
}

bool FlipSocialRun::httpRequestIsFinished()
{
    FlipSocialApp *app = static_cast<FlipSocialApp *>(appContext);
    if (!app)
    {
        FURI_LOG_E(TAG, "httpRequestIsFinished: App context is NULL");
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
    case SocialViewFeed:
        drawFeedView(canvas);
        break;
    case SocialViewMessageUsers:
        drawMessageUsersView(canvas);
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
            if (currentMenuIndex == SocialViewMessageUsers)
            {
                currentMenuIndex = SocialViewFeed;
            }
            else if (currentMenuIndex == SocialViewProfile)
            {
                currentMenuIndex = SocialViewMessageUsers;
                shouldDebounce = true;
            }
            break;
        case InputKeyUp:
        case InputKeyRight:
            if (currentMenuIndex == SocialViewFeed)
            {
                currentMenuIndex = SocialViewMessageUsers;
                shouldDebounce = true;
            }
            else if (currentMenuIndex == SocialViewMessageUsers)
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
            case SocialViewMessageUsers:
                currentView = SocialViewMessageUsers;
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
        case InputKeyLeft:
        case InputKeyDown:
            if (feedItemIndex > 0)
            {
                feedItemIndex--;
                shouldDebounce = true;
            }
            else
            {
                // If at the start of the feed, show previous page
                if (feedStatus == FeedSuccess && feedIteration > 1)
                {
                    feedIteration--;
                    feedItemIndex = MAX_FEED_ITEMS - 1;
                    // no need to load again since we already have the data
                }
            }
            break;
        case InputKeyRight:
        case InputKeyUp:
            if (feedItemIndex < (MAX_FEED_ITEMS - 1))
            {
                feedItemIndex++;
                shouldDebounce = true;
            }
            else
            {
                // If at the end of the feed, request next page
                if (feedStatus == FeedSuccess)
                {
                    feedIteration++;
                    feedItemIndex = 0;
                    feedStatus = FeedWaiting;
                    userRequest(RequestTypeFeed);
                }
            }
            break;
        case InputKeyOk:
            userRequest(RequestTypeFlipPost);
            shouldDebounce = true;
            break;
        default:
            break;
        };
        break;
    }
    case SocialViewMessageUsers:
    {
        switch (lastInput)
        {
        case InputKeyBack:
            currentView = SocialViewMenu;
            shouldDebounce = true;
            break;
        case InputKeyLeft:
        case InputKeyDown:
            if (messageUserIndex > 0)
            {
                messageUserIndex--;
                shouldDebounce = true;
            }
            break;
        case InputKeyRight:
        case InputKeyUp:
            if (messageUserIndex < (MAX_MESSAGE_USERS - 1))
            {
                messageUserIndex++;
                shouldDebounce = true;
            }
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
        switch (requestType)
        {
        case RequestTypeLogin:
            loginStatus = LoginRequestError;
            break;
        case RequestTypeRegistration:
            registrationStatus = RegistrationRequestError;
            break;
        case RequestTypeUserInfo:
            userInfoStatus = UserInfoRequestError;
            break;
        case RequestTypeFeed:
        case RequestTypeFlipPost:
        case RequestTypeCommentFetch:
            feedStatus = FeedRequestError;
            break;
        case RequestTypeMessagesUserList:
            messageUsersStatus = MessageUsersRequestError;
            break;
        default:
            break;
        }
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
        case RequestTypeFeed:
        case RequestTypeFlipPost:
        case RequestTypeCommentFetch:
            feedStatus = FeedRequestError;
            break;
        case RequestTypeMessagesUserList:
            messageUsersStatus = MessageUsersRequestError;
            break;
        default:
            FURI_LOG_E(TAG, "Unknown request type: %d", requestType);
            loginStatus = LoginRequestError;
            registrationStatus = RegistrationRequestError;
            userInfoStatus = UserInfoRequestError;
            feedStatus = FeedRequestError;
            messageUsersStatus = MessageUsersRequestError;
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
    case RequestTypeFeed:
    {
        char *url = (char *)malloc(128);
        char *feedSaveName = (char *)malloc(16);
        if (!url || !feedSaveName)
        {
            FURI_LOG_E(TAG, "userRequest: Failed to allocate memory for url");
            feedStatus = FeedRequestError;
            free(username);
            free(password);
            free(payload);
            if (feedSaveName)
                free(feedSaveName);
            if (url)
                free(url);
            return;
        }
        snprintf(url, 128, "https://www.jblanked.com/flipper/api/feed/%d/%s/%d/max/series/", MAX_FEED_ITEMS, username, feedIteration);
        snprintf(feedSaveName, 16, "feed_%d.txt", feedIteration);
        if (!app->httpRequestAsync(feedSaveName, url, GET, "{\"Content-Type\":\"application/json\"}"))
        {
            feedStatus = FeedRequestError;
        }
        free(url);
        free(feedSaveName);
        break;
    }
    case RequestTypeFlipPost:
    {
        char *feedPostPayload = (char *)malloc(256);
        if (!feedPostPayload)
        {
            FURI_LOG_E(TAG, "userRequest: Failed to allocate memory for feedPostPayload");
            feedStatus = FeedRequestError;
            free(username);
            free(password);
            free(payload);
            if (feedPostPayload)
                free(feedPostPayload);
            return;
        }
        snprintf(feedPostPayload, 256, "{\"username\":\"%s\",\"post_id\":\"%u\"}", username, feedItemID);
        if (!app->httpRequestAsync("flip_post.txt", "https://www.jblanked.com/flipper/api/feed/flip/", POST, "{\"Content-Type\":\"application/json\"}", feedPostPayload))
        {
            feedStatus = FeedRequestError;
        }
        free(feedPostPayload);
        break;
    }
    case RequestTypeCommentFetch:
    {
        char *url = (char *)malloc(128);
        char *feedSaveName = (char *)malloc(32);
        if (!url || !feedSaveName)
        {
            FURI_LOG_E(TAG, "userRequest: Failed to allocate memory for url");
            feedStatus = FeedRequestError;
            free(username);
            free(password);
            free(payload);
            if (feedSaveName)
                free(feedSaveName);
            if (url)
                free(url);
            return;
        }
        snprintf(url, 128, "https://www.jblanked.com/flipper/api/feed/comments/%d/%s/%d/", MAX_FEED_ITEMS, username, feedItemID);
        snprintf(feedSaveName, 32, "feed_%d_comments.txt", feedIteration);
        if (!app->httpRequestAsync(feedSaveName, url, GET, "{\"Content-Type\":\"application/json\"}"))
        {
            feedStatus = FeedRequestError;
        }
        free(url);
        free(feedSaveName);
        break;
    }
    case RequestTypeMessagesUserList:
    {
        char *url = (char *)malloc(128);
        char *authHeader = (char *)malloc(256);
        if (!url || !authHeader)
        {
            FURI_LOG_E(TAG, "userRequest: Failed to allocate memory for url or authHeader");
            messageUsersStatus = MessageUsersRequestError;
            free(username);
            free(password);
            free(payload);
            if (url)
                free(url);
            if (authHeader)
                free(authHeader);
            return;
        }
        snprintf(url, 128, "https://www.jblanked.com/flipper/api/messages/%s/get/list/%d/", username, MAX_MESSAGE_USERS);
        snprintf(authHeader, 256, "{\"Content-Type\":\"application/json\",\"Username\":\"%s\",\"Password\":\"%s\"}", username, password);
        if (!app->httpRequestAsync("messages_user_list.txt", url, GET, authHeader))
        {
            messageUsersStatus = MessageUsersRequestError;
        }
        free(url);
        free(authHeader);
        break;
    }
    default:
        FURI_LOG_E(TAG, "Unknown request type: %d", requestType);
        loginStatus = LoginRequestError;
        registrationStatus = RegistrationRequestError;
        userInfoStatus = UserInfoRequestError;
        feedStatus = FeedRequestError;
        messageUsersStatus = MessageUsersRequestError;
        free(username);
        free(password);
        free(payload);
        return;
    }

    free(username);
    free(password);
    free(payload);
}