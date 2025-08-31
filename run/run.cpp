#include "run/run.hpp"
#include "app.hpp"
#include <vector>
#include <flip_social_icons.h>

FlipSocialRun::FlipSocialRun(void *appContext) : appContext(appContext), commentsIndex(0), commentIsValid(false), commentItemID(0), commentsStatus(CommentsNotStarted),
                                                 currentMenuIndex(SocialViewFeed), currentProfileElement(ProfileElementBio), currentView(SocialViewLogin),
                                                 exploreIndex(0), exploreStatus(ExploreKeyboardUsers),
                                                 feedItemID(0), feedItemIndex(0), feedIteration(1), feedStatus(FeedNotStarted), inputHeld(false), lastInput(InputKeyMAX),
                                                 loginStatus(LoginNotStarted), messagesStatus(MessagesNotStarted), messageUsersStatus(MessageUsersNotStarted), messageUserIndex(0),
                                                 postStatus(PostChoose), registrationStatus(RegistrationNotStarted),
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

void FlipSocialRun::drawCommentsView(Canvas *canvas)
{
    static bool loadingStarted = false;
    switch (commentsStatus)
    {
    case CommentsWaiting:
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
                commentsStatus = CommentsRequestError;
                if (loading)
                {
                    loading->stop();
                }
                loadingStarted = false;
                return;
            }
            char *response = (char *)malloc(4096);
            char *feedSaveKey = (char *)malloc(32);
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
            snprintf(feedSaveKey, 32, "feed_%d_comments", feedIteration);
            if (app && app->loadChar(feedSaveKey, response, 4096))
            {
                commentsStatus = CommentsSuccess;
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
    case CommentsSuccess:
    {
        char *response = (char *)malloc(4096);
        char *feedSaveKey = (char *)malloc(32);
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
            canvas_draw_str(canvas, 0, 10, "Failed to load comments data.");
            return;
        }
        snprintf(feedSaveKey, 32, "feed_%d_comments", feedIteration);
        FlipSocialApp *app = static_cast<FlipSocialApp *>(appContext);
        if (app && app->loadChar(feedSaveKey, response, 4096))
        {
            if (strstr(response, "\"comments\":[{") != NULL)
            {
                commentIsValid = true;
                // Count total comments first
                int total_comments = 0;
                for (int j = 0; j < MAX_COMMENTS; j++)
                {
                    char *tempComment = get_json_array_value("comments", j, response);
                    if (tempComment)
                    {
                        total_comments++;
                        free(tempComment);
                    }
                    else
                    {
                        break;
                    }
                }

                // Parse the comments array and display the current comment at commentsIndex
                for (int i = 0; i < MAX_COMMENTS; i++)
                {
                    if (i != commentsIndex)
                    {
                        continue; // only draw the current displayed comment
                    }
                    // Get the specific comment at the current index
                    char *commentItem = get_json_array_value("comments", i, response);
                    if (commentItem)
                    {
                        char *username = get_json_value("username", commentItem);
                        char *message = get_json_value("message", commentItem);
                        char *flipped = get_json_value("flipped", commentItem);
                        char *flips_str = get_json_value("flip_count", commentItem);
                        char *date_created = get_json_value("date_created", commentItem);
                        char *id_str = get_json_value("id", commentItem);
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
                            free(commentItem);
                            commentsStatus = CommentsParseError;
                            return;
                        }
                        commentItemID = atoi(id_str);
                        drawFeedItem(canvas, username, message, flipped, flips_str, date_created);

                        // Draw navigation arrows if there are multiple comments
                        canvas_set_font_custom(canvas, FONT_SIZE_SMALL);
                        if (commentsIndex > 0)
                        {
                            canvas_draw_str(canvas, 2, 60, "< Prev");
                        }
                        if (commentsIndex < (total_comments - 1))
                        {
                            canvas_draw_str(canvas, 96, 60, "Next >");
                        }

                        // Draw comment counter
                        char comment_counter[32];
                        snprintf(comment_counter, sizeof(comment_counter), "%d/%d", commentsIndex + 1, total_comments);
                        canvas_draw_str(canvas, 112, 10, comment_counter);

                        free(username);
                        free(message);
                        free(flipped);
                        free(flips_str);
                        free(date_created);
                        free(id_str);
                        free(commentItem);
                        break; // Exit the loop after drawing the current comment
                    }
                    else
                    {
                        // If current comment index doesn't exist, go back to previous
                        if (commentsIndex > 0)
                        {
                            commentsIndex--;
                        }
                        break;
                    }
                }
            }
            else
            {
                canvas_set_font_custom(canvas, FONT_SIZE_SMALL);
                canvas_draw_str(canvas, 0, 10, "No comments found for this post.");
                canvas_draw_str(canvas, 0, 60, "Be the first, click DOWN");
            }
        }

        free(response);
        free(feedSaveKey);
        break;
    }
    case CommentsRequestError:
        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 0, 10, "Comments request failed!");
        canvas_draw_str(canvas, 0, 20, "Check your network and");
        canvas_draw_str(canvas, 0, 30, "try again later.");
        break;
    case CommentsParseError:
        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 0, 10, "Failed to parse comments!");
        canvas_draw_str(canvas, 0, 20, "Try again...");
        break;
    case CommentsNotStarted:
        canvas_clear(canvas);
        commentsStatus = CommentsWaiting;
        userRequest(RequestTypeCommentFetch);
        break;
    case CommentsKeyboard:
        if (!keyboard)
        {
            keyboard = std::make_unique<Keyboard>();
        }
        if (keyboard)
        {
            keyboard->draw(canvas, "Comment:");
        }
        break;
    case CommentsSending:
        if (!loadingStarted)
        {
            if (!loading)
            {
                loading = std::make_unique<Loading>(canvas);
            }
            loadingStarted = true;
            if (loading)
            {
                loading->setText("Sending...");
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
                commentsStatus = CommentsRequestError;
                return;
            }
            char *response = (char *)malloc(64);
            if (response && app->loadChar("comment_post", response, 64) && strstr(response, "[SUCCESS]") != NULL)
            {
                currentView = SocialViewFeed;
                currentMenuIndex = SocialViewFeed;
                commentsStatus = CommentsNotStarted;
                feedStatus = FeedNotStarted;
                feedItemIndex = 0;
                feedIteration = 1;
                free(response);
                return;
            }
            else
            {
                commentsStatus = CommentsRequestError;
            }
        }
        break;
    default:
        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 0, 10, "Loading comments...");
        break;
    }
}

void FlipSocialRun::drawExploreView(Canvas *canvas)
{
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    static bool loadingStarted = false;
    switch (exploreStatus)
    {
    case ExploreWaiting:
        if (!loadingStarted)
        {
            if (!loading)
            {
                loading = std::make_unique<Loading>(canvas);
            }
            loadingStarted = true;
            if (loading)
            {
                loading->setText("Searching...");
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
                exploreStatus = ExploreRequestError;
                return;
            }
            char *response = (char *)malloc(1024);
            if (response && app->loadChar("explore", response, 1024) && strstr(response, "users") != NULL)
            {
                exploreStatus = ExploreSuccess;
                free(response);
                return;
            }
            else
            {
                exploreStatus = ExploreRequestError;
            }
        }
        break;
    case ExploreSuccess:
    {
        canvas_draw_str(canvas, 0, 10, "Explore success!");
        canvas_draw_str(canvas, 0, 20, "Press OK to continue.");
        char *messagesUserList = (char *)malloc(1024);
        if (!messagesUserList)
        {
            FURI_LOG_E(TAG, "drawExploreView: Failed to allocate memory for messagesUserList");
            exploreStatus = ExploreParseError;
            return;
        }

        FlipSocialApp *app = static_cast<FlipSocialApp *>(appContext);
        if (!app || !app->loadChar("explore", messagesUserList, 1024))
        {
            FURI_LOG_E(TAG, "drawExploreView: Failed to load explore data from storage");
            canvas_draw_str(canvas, 0, 30, "Failed to load explore data.");
            free(messagesUserList);
            return;
        }

        // store users
        std::vector<std::string> usersList;
        for (int i = 0; i < MAX_EXPLORE_USERS; i++)
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
            canvas_draw_str(canvas, 0, 30, "No users found.");
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
            drawMenu(canvas, exploreIndex, userPtrs.data(), userPtrs.size());
        }

        free(messagesUserList);
        break;
    }
    case ExploreRequestError:
        canvas_draw_str(canvas, 0, 10, "Messages request failed!");
        canvas_draw_str(canvas, 0, 20, "Check your network and");
        canvas_draw_str(canvas, 0, 30, "try again later.");
        break;
    case ExploreParseError:
        canvas_draw_str(canvas, 0, 10, "Error parsing messages!");
        canvas_draw_str(canvas, 0, 20, "Please set your username");
        canvas_draw_str(canvas, 0, 30, "and password in the app.");
        break;
    case ExploreNotStarted:
        exploreStatus = ExploreWaiting;
        userRequest(RequestTypeExplore);
        break;
    case ExploreKeyboardUsers:
        if (!keyboard)
        {
            keyboard = std::make_unique<Keyboard>();
        }
        if (keyboard)
        {
            keyboard->draw(canvas, "Enter text:");
        }
        break;
    case ExploreKeyboardMessage:
        if (!keyboard)
        {
            keyboard = std::make_unique<Keyboard>();
        }
        if (keyboard)
        {
            keyboard->draw(canvas, "Enter message:");
        }
        break;
    case ExploreSending:
        if (!loadingStarted)
        {
            if (!loading)
            {
                loading = std::make_unique<Loading>(canvas);
            }
            loadingStarted = true;
            if (loading)
            {
                loading->setText("Sending...");
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
                exploreStatus = ExploreRequestError;
                return;
            }
            char *response = (char *)malloc(64);
            if (response && app->loadChar("messages_post", response, 64) && strstr(response, "[SUCCESS]") != NULL)
            {
                currentView = SocialViewMessageUsers;
                currentMenuIndex = SocialViewMessageUsers;
                messageUsersStatus = MessageUsersNotStarted;
                exploreStatus = ExploreKeyboardUsers;
                exploreIndex = 0;
                free(response);
                return;
            }
            else
            {
                exploreStatus = ExploreRequestError;
            }
        }
        break;
    default:
        canvas_draw_str(canvas, 0, 10, "Retrieving messages...");
        break;
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
    canvas_draw_icon(canvas, 35, 54, &I_ButtonOK_7x7);
    char flip_status[16];
    snprintf(flip_status, sizeof(flip_status), isFlipped ? "Unflip" : "Flip");
    canvas_draw_str(canvas, isFlipped ? 44 : 46, 60, flip_status);
    if (strstr(date_created, "minutes ago") == NULL)
    {
        canvas_draw_str(canvas, 76, 60, date_created);
    }
    else
    {
        canvas_draw_str(canvas, 72, 60, date_created);
    }
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
                        feedStatus = FeedParseError;
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
    const char *menuItems[] = {"Feed", "Post", "Messages", "Explore", "Profile"};
    drawMenu(canvas, (uint8_t)currentMenuIndex, menuItems, 5);
}

void FlipSocialRun::drawMenu(Canvas *canvas, uint8_t selectedIndex, const char **menuItems, uint8_t menuCount)
{
    canvas_clear(canvas);

    // Draw title
    canvas_set_font_custom(canvas, FONT_SIZE_LARGE);
    const char *title = "FlipSocial";
    int title_width = canvas_string_width(canvas, title);
    int title_x = (128 - title_width) / 2;
    canvas_draw_str(canvas, title_x, 12, title);

    // Draw underline for title
    canvas_draw_line(canvas, title_x, 14, title_x + title_width, 14);

    // Draw decorative horizontal pattern
    for (int i = 0; i < 128; i += 4)
    {
        canvas_draw_dot(canvas, i, 18);
    }

    // Menu items with word wrapping
    canvas_set_font_custom(canvas, FONT_SIZE_MEDIUM);
    const char *currentItem = menuItems[selectedIndex];

    const int box_padding = 10;
    const int box_width = 108;
    const int usable_width = box_width - (box_padding * 2); // Text area inside box
    const int line_height = 8;                              // Typical line height for medium font
    const int max_lines = 2;                                // Maximum lines to prevent overflow

    int menu_y = 40;

    // Calculate word wrapping
    char lines[max_lines][64];
    int line_count = 0;

    // word wrap
    const char *text = currentItem;
    int text_len = strlen(text);
    int current_pos = 0;

    while (current_pos < text_len && line_count < max_lines)
    {
        int line_start = current_pos;
        int last_space = -1;
        int current_width = 0;
        int char_pos = 0;

        // Find how much text fits on this line
        while (current_pos < text_len && char_pos < 63) // Leave room for null terminator
        {
            if (text[current_pos] == ' ')
            {
                last_space = char_pos;
            }

            lines[line_count][char_pos] = text[current_pos];
            char_pos++;

            // Check if adding this character exceeds width
            lines[line_count][char_pos] = '\0'; // Temporary null terminator
            current_width = canvas_string_width(canvas, lines[line_count]);

            if (current_width > usable_width)
            {
                // Text is too wide, need to break
                if (last_space > 0)
                {
                    // Break at last space
                    lines[line_count][last_space] = '\0';
                    current_pos = line_start + last_space + 1; // Skip the space
                }
                else
                {
                    // No space found, break at previous character
                    char_pos--;
                    lines[line_count][char_pos] = '\0';
                    current_pos = line_start + char_pos;
                }
                break;
            }

            current_pos++;
        }

        // If we reached end of text
        if (current_pos >= text_len)
        {
            lines[line_count][char_pos] = '\0';
        }

        line_count++;
    }

    // If there's still more text and we're at max lines, add ellipsis
    if (current_pos < text_len && line_count == max_lines)
    {
        int last_line = line_count - 1;
        int line_len = strlen(lines[last_line]);
        if (line_len > 3)
        {
            strcpy(&lines[last_line][line_len - 3], "...");
        }
    }

    // Calculate box height based on number of lines, but keep minimum height
    int box_height = (line_count * line_height) + 8;
    if (box_height < 16)
        box_height = 16;

    // Dynamic box positioning based on content height
    int box_y_offset;
    if (line_count > 1)
    {
        box_y_offset = -22;
    }
    else
    {
        box_y_offset = -12;
    }

    // Draw main selection box
    canvas_draw_rbox(canvas, 10, menu_y + box_y_offset, box_width, box_height, 4);
    canvas_set_color(canvas, ColorWhite);

    // Draw each line of text centered
    for (int i = 0; i < line_count; i++)
    {
        int line_width = canvas_string_width(canvas, lines[i]);
        int line_x = (128 - line_width) / 2;
        int text_y_offset = (line_count > 1) ? -18 : -4;
        int line_y = menu_y + (i * line_height) + 4 + text_y_offset;
        canvas_draw_str(canvas, line_x, line_y, lines[i]);
    }

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

    const int MAX_DOTS = 15;
    const int dots_spacing = 6;
    int indicator_y = 52;

    if (menuCount <= MAX_DOTS)
    {
        // Show all dots if they fit
        int dots_start_x = (128 - (menuCount * dots_spacing)) / 2;
        for (int i = 0; i < menuCount; i++)
        {
            int dot_x = dots_start_x + (i * dots_spacing);
            if (i == selectedIndex)
            {
                canvas_draw_box(canvas, dot_x, indicator_y, 4, 4);
            }
            else
            {
                canvas_draw_frame(canvas, dot_x, indicator_y, 4, 4);
            }
        }
    }
    else
    {
        // condensed indicator with current position
        canvas_set_font_custom(canvas, FONT_SIZE_SMALL);
        char position_text[16];
        snprintf(position_text, sizeof(position_text), "%d/%d", selectedIndex + 1, menuCount);
        int pos_width = canvas_string_width(canvas, position_text);
        int pos_x = (128 - pos_width) / 2;
        canvas_draw_str(canvas, pos_x, indicator_y + 3, position_text);

        // progress bar
        int bar_width = 60;
        int bar_x = (128 - bar_width) / 2;
        int bar_y = indicator_y - 6;
        canvas_draw_frame(canvas, bar_x, bar_y, bar_width, 3);
        int progress_width = (selectedIndex * (bar_width - 2)) / (menuCount - 1);
        canvas_draw_box(canvas, bar_x + 1, bar_y + 1, progress_width, 1);
    }

    // Draw decorative bottom pattern
    for (int i = 0; i < 128; i += 4)
    {
        canvas_draw_dot(canvas, i, 58);
    }
}

void FlipSocialRun::drawMessagesView(Canvas *canvas)
{
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    static bool loadingStarted = false;
    switch (messagesStatus)
    {
    case MessagesWaiting:
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
                messagesStatus = MessagesRequestError;
                return;
            }
            char *response = (char *)malloc(1024);
            if (response && app->loadChar("messages_with_user", response, 1024) && strstr(response, "conversations") != NULL)
            {
                messagesStatus = MessagesSuccess;
                free(response);
                return;
            }
            else
            {
                messagesStatus = MessagesRequestError;
            }
        }
        break;
    case MessagesSuccess:
    {
        char *messagesUserList = (char *)malloc(1024);
        if (!messagesUserList)
        {
            FURI_LOG_E(TAG, "drawMessageUsersView: Failed to allocate memory for messagesUserList");
            messagesStatus = MessagesParseError;
            return;
        }

        FlipSocialApp *app = static_cast<FlipSocialApp *>(appContext);
        if (!app || !app->loadChar("messages_with_user", messagesUserList, 1024))
        {
            FURI_LOG_E(TAG, "drawMessageUsersView: Failed to load messages user list from storage");
            canvas_draw_str(canvas, 0, 30, "Failed to load messages.");
            free(messagesUserList);
            return;
        }

        // draw the current message
        for (int i = 0; i < MAX_MESSAGES; i++)
        {
            if (i != messagesIndex)
            {
                continue; // only draw the current displayed message
            }
            char *message = get_json_array_value("conversations", i, messagesUserList);
            if (!message)
            {
                FURI_LOG_E(TAG, "drawMessagesView: Failed to get message from JSON");
                free(messagesUserList);
                if (messagesIndex > 0)
                {
                    // go back to the previous message if current is not found
                    messagesIndex--;
                }
                return;
            }
            char *sender = get_json_value("sender", message);
            char *content = get_json_value("content", message);
            if (!sender || !content)
            {
                FURI_LOG_E(TAG, "drawMessagesView: Failed to parse message data");
                free(sender);
                free(content);
                free(message);
                free(messagesUserList);
                return;
            }

            // sender name
            canvas_set_font_custom(canvas, FONT_SIZE_LARGE);
            canvas_draw_str(canvas, 0, 10, sender);

            // underline for sender
            int sender_width = canvas_string_width(canvas, sender);
            canvas_draw_line(canvas, 0, 12, sender_width, 12);

            // Draw message content with word wrapping
            canvas_set_font_custom(canvas, FONT_SIZE_MEDIUM);
            drawFeedMessage(canvas, content, 0, 18);

            // Draw navigation arrows if there are multiple messages
            canvas_set_font_custom(canvas, FONT_SIZE_SMALL);
            if (messagesIndex > 0)
            {
                canvas_draw_str(canvas, 2, 60, "< Prev");
            }
            if (messagesIndex < (MAX_MESSAGES - 1))
            {
                // Check if there's a next message available
                char *nextMessage = get_json_array_value("conversations", messagesIndex + 1, messagesUserList);
                if (nextMessage)
                {
                    canvas_draw_str(canvas, 96, 60, "Next >");
                    free(nextMessage);
                }
            }

            // Draw message counter
            char message_counter[32];
            int total_messages = 0;
            // Count total messages
            for (int j = 0; j < MAX_MESSAGES; j++)
            {
                char *tempMessage = get_json_array_value("conversations", j, messagesUserList);
                if (tempMessage)
                {
                    total_messages++;
                    free(tempMessage);
                }
                else
                {
                    break;
                }
            }
            snprintf(message_counter, sizeof(message_counter), "%d/%d", messagesIndex + 1, total_messages);
            canvas_draw_str(canvas, 112, 10, message_counter);

            canvas_draw_icon(canvas, 52, 54, &I_ButtonOK_7x7);
            canvas_draw_str(canvas, 60, 60, "Reply");

            free(sender);
            free(content);
            free(message);
        }

        free(messagesUserList);
        break;
    }
    case MessagesRequestError:
        canvas_draw_str(canvas, 0, 10, "Messages request failed!");
        canvas_draw_str(canvas, 0, 20, "Check your network and");
        canvas_draw_str(canvas, 0, 30, "try again later.");
        break;
    case MessagesParseError:
        canvas_draw_str(canvas, 0, 10, "Error parsing messages!");
        canvas_draw_str(canvas, 0, 20, "Please set your username");
        canvas_draw_str(canvas, 0, 30, "and password in the app.");
        break;
    case MessagesNotStarted:
        messagesStatus = MessagesWaiting;
        userRequest(RequestTypeMessagesWithUser);
        break;
    case MessagesKeyboard:
        if (!keyboard)
        {
            keyboard = std::make_unique<Keyboard>();
        }
        if (keyboard)
        {
            keyboard->draw(canvas, "Enter reply:");
        }
        break;
    case MessagesSending:
        if (!loadingStarted)
        {
            if (!loading)
            {
                loading = std::make_unique<Loading>(canvas);
            }
            loadingStarted = true;
            if (loading)
            {
                loading->setText("Sending...");
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
                messagesStatus = MessagesRequestError;
                return;
            }
            char *response = (char *)malloc(64);
            if (response && app->loadChar("messages_post", response, 64) && strstr(response, "[SUCCESS]") != NULL)
            {
                messagesStatus = MessagesNotStarted;
                messagesIndex = 0;
                free(response);
                return;
            }
            else
            {
                messagesStatus = MessagesRequestError;
            }
        }
        break;
    default:
        canvas_draw_str(canvas, 0, 10, "Retrieving messages...");
        break;
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

void FlipSocialRun::drawPostView(Canvas *canvas)
{
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    static bool loadingStarted = false;
    switch (postStatus)
    {
    case PostWaiting:
        if (!loadingStarted)
        {
            if (!loading)
            {
                loading = std::make_unique<Loading>(canvas);
            }
            loadingStarted = true;
            if (loading)
            {
                loading->setText("Posting...");
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
                postStatus = PostRequestError;
                return;
            }
            char *response = (char *)malloc(128);
            if (response && app->loadChar("post", response, 128) && strstr(response, "[SUCCESS]") != NULL)
            {
                postStatus = PostSuccess;
                currentView = SocialViewFeed;
                currentMenuIndex = SocialViewFeed;
                feedStatus = FeedNotStarted;
                feedIteration = 1;
                feedItemIndex = 0;
                free(response);
                return;
            }
            else
            {
                postStatus = PostRequestError;
            }
        }
        break;
    case PostSuccess:
        // unlike other "views", we shouldnt hit here
        // since after posting, users will be redirected to feed
        canvas_draw_str(canvas, 0, 10, "Posted successfully!");
        canvas_draw_str(canvas, 0, 20, "Press OK to continue.");
        break;
    case PostRequestError:
        canvas_draw_str(canvas, 0, 10, "Post request failed!");
        canvas_draw_str(canvas, 0, 20, "Ensure your message");
        canvas_draw_str(canvas, 0, 30, "follows the rules.");
        break;
    case PostParseError:
        canvas_draw_str(canvas, 0, 10, "Error parsing post!");
        canvas_draw_str(canvas, 0, 20, "Ensure your message");
        canvas_draw_str(canvas, 0, 30, "follows the rules.");
        break;
    case PostKeyboard:
        if (!keyboard)
        {
            keyboard = std::make_unique<Keyboard>();
        }
        if (keyboard)
        {
            keyboard->draw(canvas, "Enter post:");
        }
        break;
    case PostChoose:
    {
        char *preSavedLocation = (char *)malloc(128);
        char *preSavedMessages = (char *)malloc(1024);
        if (!preSavedLocation || !preSavedMessages)
        {
            FURI_LOG_E(TAG, "drawPostView: Failed to allocate memory for preSavedLocation or preSavedMessages");
            postStatus = PostParseError;
            return;
        }
        snprintf(preSavedLocation, 128, "%s/apps_data/flip_social/pre_saved_messages.txt", STORAGE_EXT_PATH_PREFIX);
        FlipSocialApp *app = static_cast<FlipSocialApp *>(appContext);
        if (!app || !app->loadFileChunk(preSavedLocation, preSavedMessages, 1024, 0))
        {
            FURI_LOG_E(TAG, "drawPostView: Failed to load pre-saved messages from file");
            // canvas_draw_str(canvas, 0, 10, "Failed to load pre-saved messages.");
            // free(preSavedLocation);
            // free(preSavedMessages);
            // return;
            // we should create one instead and then return if failed to create
        }

        // Parse pre-saved messages (each line is a message)
        std::vector<std::string> preSavedList;
        char *start = preSavedMessages;
        char *end = preSavedMessages;
        int preSavedCount = 0;
        while (*end != '\0' && preSavedCount < MAX_PRE_SAVED_MESSAGES)
        {
            if (*end == '\n')
            {
                if (end > start)
                {
                    preSavedList.push_back(std::string(start, end - start));
                    preSavedCount++;
                }
                end++;
                start = end;
            }
            else
            {
                end++;
            }
        }
        // Add the last line if not empty and not ending with a newline, and if limit not reached
        if (end > start && preSavedCount < MAX_PRE_SAVED_MESSAGES)
        {
            preSavedList.push_back(std::string(start, end - start));
        }

        // Insert "[New Post]" as the first item, then add user's pre-saved messages
        std::vector<std::string> menuItems;
        menuItems.push_back("[New Post]");
        for (const auto &msg : preSavedList)
        {
            menuItems.push_back(msg);
        }

        // Convert std::vector<std::string> to const char** for drawMenu
        std::vector<const char *> preSavedPtrs;
        preSavedPtrs.reserve(menuItems.size());
        for (const auto &msg : menuItems)
        {
            preSavedPtrs.push_back(msg.c_str());
        }

        // Draw the menu with [New Post] and pre-saved messages
        drawMenu(canvas, postIndex, preSavedPtrs.data(), preSavedPtrs.size());

        free(preSavedLocation);
        free(preSavedMessages);
        break;
    }
    default:
        canvas_draw_str(canvas, 0, 10, "Awaiting...");
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

bool FlipSocialRun::getMessageUser(char *buffer, size_t buffer_size)
{
    char *messagesUserList = (char *)malloc(1024);
    uint8_t index = currentMenuIndex == SocialViewExplore ? exploreIndex : messageUserIndex;
    FlipSocialApp *app = static_cast<FlipSocialApp *>(appContext);
    if (!app || !messagesUserList || !app->loadChar(currentMenuIndex == SocialViewExplore ? "explore" : "messages_user_list", messagesUserList, 1024))
    {
        FURI_LOG_E(TAG, "getMessageUser: Failed to load messages user list from storage");
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

bool FlipSocialRun::getSelectedPost(char *buffer, size_t buffer_size)
{
    if (postIndex == 0)
    {
        // If postIndex is 0, we are in the "New Post" mode
        snprintf(buffer, buffer_size, "[New Post]");
        return true;
    }
    char *preSavedLocation = (char *)malloc(128);
    char *preSavedMessages = (char *)malloc(1024);
    if (!preSavedLocation || !preSavedMessages)
    {
        FURI_LOG_E(TAG, "getSelectedPost: Failed to allocate memory for preSavedLocation or preSavedMessages");
        return false;
    }
    snprintf(preSavedLocation, 128, "%s/apps_data/flip_social/pre_saved_messages.txt", STORAGE_EXT_PATH_PREFIX);
    FlipSocialApp *app = static_cast<FlipSocialApp *>(appContext);
    if (!app || !app->loadFileChunk(preSavedLocation, preSavedMessages, 1024, 0))
    {
        FURI_LOG_E(TAG, "getSelectedPost: Failed to load pre-saved messages from file");
        free(preSavedLocation);
        free(preSavedMessages);
        return false;
    }

    // grab only the selected post
    char *start = preSavedMessages;
    char *end = preSavedMessages;
    int postCount = 0;
    while (*end != '\0')
    {
        if (*end == '\n')
        {
            if (postCount == postIndex - 1)
            {
                // Found the selected post
                size_t length = end - start;
                if (length < buffer_size)
                {
                    snprintf(buffer, buffer_size, "%.*s", (int)length, start);
                    free(preSavedLocation);
                    free(preSavedMessages);
                    return true;
                }
                else
                {
                    FURI_LOG_E(TAG, "getSelectedPost: Buffer size is too small for the post");
                    free(preSavedLocation);
                    free(preSavedMessages);
                    return false;
                }
            }
            postCount++;
            start = end + 1; // Move to the next line
        }
        end++;
    }

    free(preSavedLocation);
    free(preSavedMessages);
    return false; // Post not found
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
    case SocialViewMessages:
        drawMessagesView(canvas);
        break;
    case SocialViewExplore:
        drawExploreView(canvas);
        break;
    case SocialViewPost:
        drawPostView(canvas);
        break;
    case SocialViewComments:
        drawCommentsView(canvas);
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
            if (currentMenuIndex == SocialViewPost)
            {
                currentMenuIndex = SocialViewFeed;
            }
            else if (currentMenuIndex == SocialViewMessageUsers)
            {
                currentMenuIndex = SocialViewPost;
                shouldDebounce = true;
            }
            else if (currentMenuIndex == SocialViewExplore)
            {
                currentMenuIndex = SocialViewMessageUsers;
                shouldDebounce = true;
            }
            else if (currentMenuIndex == SocialViewProfile)
            {
                currentMenuIndex = SocialViewExplore;
                shouldDebounce = true;
            }
            break;
        case InputKeyUp:
        case InputKeyRight:
            if (currentMenuIndex == SocialViewFeed)
            {
                currentMenuIndex = SocialViewPost;
                shouldDebounce = true;
            }
            else if (currentMenuIndex == SocialViewPost)
            {
                currentMenuIndex = SocialViewMessageUsers;
                shouldDebounce = true;
            }
            else if (currentMenuIndex == SocialViewMessageUsers)
            {
                currentMenuIndex = SocialViewExplore;
                shouldDebounce = true;
            }
            else if (currentMenuIndex == SocialViewExplore)
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
            case SocialViewPost:
                currentView = SocialViewPost;
                shouldDebounce = true;
                break;
            case SocialViewMessageUsers:
                currentView = SocialViewMessageUsers;
                shouldDebounce = true;
                break;
            case SocialViewExplore:
                currentView = SocialViewExplore;
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
        case InputKeyDown:
            // Switch to comments view for current feed item
            currentView = SocialViewComments;
            commentsStatus = CommentsNotStarted;
            commentsIndex = 0;
            shouldDebounce = true;
            break;
        case InputKeyLeft:
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
    case SocialViewPost:
    {
        if (postStatus == PostKeyboard)
        {
            if (keyboard)
            {
                if (keyboard->handleInput(lastInput))
                {
                    FlipSocialApp *app = static_cast<FlipSocialApp *>(appContext);
                    app->saveChar("new_feed_post", keyboard->getText());
                    postStatus = PostWaiting;
                    userRequest(RequestTypePost);
                }
                if (lastInput != InputKeyMAX)
                {
                    shouldDebounce = true;
                }
            }
            if (lastInput == InputKeyBack)
            {
                postStatus = PostChoose;
                shouldDebounce = true;
                if (keyboard)
                {
                    keyboard->clearText();
                    keyboard.reset();
                }
            }
        }
        else
        {
            switch (lastInput)
            {
            case InputKeyBack:
                currentView = SocialViewMenu;
                shouldDebounce = true;
                break;
            case InputKeyLeft:
            case InputKeyDown:
                if (postIndex > 0)
                {
                    postIndex--;
                    shouldDebounce = true;
                }
                break;
            case InputKeyRight:
            case InputKeyUp:
                if (postIndex < (MAX_PRE_SAVED_MESSAGES - 1))
                {
                    postIndex++;
                    shouldDebounce = true;
                }
                break;
            case InputKeyOk:
                if (postIndex == 0) // New Post
                {
                    postStatus = PostKeyboard;
                    shouldDebounce = true;
                    if (keyboard)
                    {
                        keyboard->clearText();
                        keyboard.reset();
                    }
                }
                else
                {
                    char *selectedPost = (char *)malloc(128);
                    if (!selectedPost)
                    {
                        FURI_LOG_E(TAG, "updateInput: Failed to allocate memory for selectedPost");
                        postStatus = PostParseError;
                        shouldDebounce = true;
                        return;
                    }
                    if (getSelectedPost(selectedPost, 128))
                    {
                        if (!keyboard)
                        {
                            keyboard = std::make_unique<Keyboard>();
                        }
                        if (keyboard)
                        {
                            keyboard->setText(selectedPost);
                            postStatus = PostKeyboard;
                            shouldDebounce = true;
                        }
                    }
                    free(selectedPost);
                }
                break;
            default:
                break;
            };
        }
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
        case InputKeyOk:
            currentView = SocialViewMessages;
            shouldDebounce = true;
            break;
        default:
            break;
        };
        break;
    }
    case SocialViewMessages:
    {
        if (messagesStatus == MessagesKeyboard)
        {
            if (keyboard)
            {
                if (keyboard->handleInput(lastInput))
                {
                    FlipSocialApp *app = static_cast<FlipSocialApp *>(appContext);
                    app->saveChar("message_to_user", keyboard->getText());
                    messagesStatus = MessagesSending;
                    userRequest(RequestTypeMessageSend);
                }
                if (lastInput != InputKeyMAX)
                {
                    shouldDebounce = true;
                }
            }
            if (lastInput == InputKeyBack)
            {
                messagesStatus = MessagesSuccess;
                shouldDebounce = true;
                if (keyboard)
                {
                    keyboard->clearText();
                    keyboard.reset();
                }
            }
        }
        else
        {
            switch (lastInput)
            {
            case InputKeyBack:
                currentView = SocialViewMessageUsers;
                messagesStatus = MessagesNotStarted;
                messagesIndex = 0;
                shouldDebounce = true;
                break;
            case InputKeyLeft:
            case InputKeyDown:
                // Navigate to previous message
                if (messagesIndex > 0)
                {
                    messagesIndex--;
                    shouldDebounce = true;
                }
                break;
            case InputKeyRight:
            case InputKeyUp:
                // Navigate to next message
                if (messagesIndex < (MAX_MESSAGES - 1))
                {
                    messagesIndex++;
                    shouldDebounce = true;
                }
                break;
            case InputKeyOk:
                messagesStatus = MessagesKeyboard;
                shouldDebounce = true;
                return;
            default:
                break;
            };
        }
        break;
    }
    case SocialViewExplore:
    {
        if (exploreStatus == ExploreKeyboardUsers)
        {
            if (keyboard)
            {
                if (keyboard->handleInput(lastInput))
                {
                    FlipSocialApp *app = static_cast<FlipSocialApp *>(appContext);
                    app->saveChar("explore_keyword", keyboard->getText());
                    exploreStatus = ExploreWaiting;
                    exploreIndex = 0;
                    userRequest(RequestTypeExplore);
                }
                if (lastInput != InputKeyMAX)
                {
                    shouldDebounce = true;
                }
            }
            if (lastInput == InputKeyBack)
            {
                currentView = SocialViewMenu;
                exploreStatus = ExploreKeyboardUsers;
                exploreIndex = 0;
                shouldDebounce = true;
                if (keyboard)
                {
                    keyboard->clearText();
                    keyboard.reset();
                }
            }
        }
        else if (exploreStatus == ExploreKeyboardMessage)
        {
            if (keyboard)
            {
                if (keyboard->handleInput(lastInput))
                {
                    FlipSocialApp *app = static_cast<FlipSocialApp *>(appContext);
                    app->saveChar("message_to_user", keyboard->getText());
                    exploreStatus = ExploreSending;
                    userRequest(RequestTypeMessageSend);
                }
                if (lastInput != InputKeyMAX)
                {
                    shouldDebounce = true;
                }
            }
            if (lastInput == InputKeyBack)
            {
                exploreStatus = ExploreSuccess;
                shouldDebounce = true;
                if (keyboard)
                {
                    keyboard->clearText();
                    keyboard.reset();
                }
            }
        }
        else
        {
            switch (lastInput)
            {
            case InputKeyBack:
                currentView = SocialViewMenu;
                exploreStatus = ExploreKeyboardUsers;
                exploreIndex = 0;
                shouldDebounce = true;
                if (keyboard)
                {
                    keyboard->clearText();
                    keyboard.reset();
                }
                break;
            case InputKeyLeft:
            case InputKeyDown:
                if (exploreIndex > 0)
                {
                    exploreIndex--;
                    shouldDebounce = true;
                }
                break;
            case InputKeyRight:
            case InputKeyUp:
                if (exploreIndex < (MAX_EXPLORE_USERS - 1))
                {
                    exploreIndex++;
                    shouldDebounce = true;
                }
                break;
            case InputKeyOk:
                exploreStatus = ExploreKeyboardMessage;
                shouldDebounce = true;
                if (keyboard)
                {
                    keyboard->clearText();
                    keyboard.reset();
                }
                return;
            default:
                break;
            };
        }
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
    case SocialViewComments:
    {
        if (commentsStatus == CommentsKeyboard)
        {
            if (keyboard)
            {
                if (keyboard->handleInput(lastInput))
                {
                    FlipSocialApp *app = static_cast<FlipSocialApp *>(appContext);
                    app->saveChar("new_comment", keyboard->getText());
                    commentsStatus = CommentsSending;
                    userRequest(RequestTypeCommentPost);
                }
                if (lastInput != InputKeyMAX)
                {
                    shouldDebounce = true;
                }
            }
            if (lastInput == InputKeyBack)
            {
                commentsStatus = CommentsSuccess;
                shouldDebounce = true;
                if (keyboard)
                {
                    keyboard->clearText();
                    keyboard.reset();
                }
            }
        }
        else
        {
            switch (lastInput)
            {
            case InputKeyBack:
                currentView = SocialViewFeed;
                commentIsValid = false;
                shouldDebounce = true;
                break;
            case InputKeyLeft:
                if (commentsIndex > 0)
                {
                    commentsIndex--;
                    shouldDebounce = true;
                }
                break;
            case InputKeyRight:
                if (commentsIndex < (MAX_COMMENTS - 1))
                {
                    commentsIndex++;
                    shouldDebounce = true;
                }
                break;
            case InputKeyDown:
                // Start reply to comment
                commentsStatus = CommentsKeyboard;
                if (!keyboard)
                {
                    keyboard = std::make_unique<Keyboard>();
                }
                if (keyboard)
                {
                    keyboard->clearText();
                    keyboard->setText(""); // Start with empty text for reply
                }
                shouldDebounce = true;
                break;
            case InputKeyOk:
                // Flip the current comment
                if (commentIsValid)
                {
                    userRequest(RequestTypeCommentFlip);
                }
                shouldDebounce = true;
                break;
            default:
                break;
            }
        }
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
        case RequestTypeCommentFlip:
        case RequestTypeCommentPost:
            feedStatus = FeedRequestError;
            break;
        case RequestTypeMessagesUserList:
            messageUsersStatus = MessageUsersRequestError;
            break;
        case RequestTypeMessagesWithUser:
            messagesStatus = MessagesRequestError;
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
        case RequestTypeCommentFlip:
        case RequestTypeCommentPost:
            feedStatus = FeedRequestError;
            break;
        case RequestTypeMessagesUserList:
            messageUsersStatus = MessageUsersRequestError;
            break;
        case RequestTypeMessagesWithUser:
            messagesStatus = MessagesRequestError;
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
        commentIsValid = false;
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
    case RequestTypeCommentFlip:
    {
        char *commentFlipPayload = (char *)malloc(256);
        if (!commentFlipPayload)
        {
            FURI_LOG_E(TAG, "userRequest: Failed to allocate memory for commentFlipPayload");
            feedStatus = FeedRequestError;
            free(username);
            free(password);
            free(payload);
            return;
        }
        snprintf(commentFlipPayload, 256, "{\"username\":\"%s\",\"post_id\":\"%u\"}", username, commentItemID);
        if (!app->httpRequestAsync("flip_comment.txt", "https://www.jblanked.com/flipper/api/feed/flip/", POST, "{\"Content-Type\":\"application/json\"}", commentFlipPayload))
        {
            feedStatus = FeedRequestError;
        }
        free(commentFlipPayload);
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
    case RequestTypeMessagesWithUser:
    {
        char *url = (char *)malloc(128);
        char *authHeader = (char *)malloc(256);
        char *messageUser = (char *)malloc(64);
        if (!url || !authHeader || !messageUser)
        {
            FURI_LOG_E(TAG, "userRequest: Failed to allocate memory for url, authHeader or messageUser");
            messagesStatus = MessagesRequestError;
            free(username);
            free(password);
            free(payload);
            if (url)
                free(url);
            if (authHeader)
                free(authHeader);
            if (messageUser)
                free(messageUser);
            return;
        }
        if (!getMessageUser(messageUser, 64))
        {
            FURI_LOG_E(TAG, "userRequest: Failed to get message user");
            messagesStatus = MessagesParseError;
            free(username);
            free(password);
            free(payload);
            free(url);
            free(authHeader);
            free(messageUser);
            return;
        }
        snprintf(authHeader, 256, "{\"Content-Type\":\"application/json\",\"Username\":\"%s\",\"Password\":\"%s\"}", username, password);
        snprintf(url, 128, "https://www.jblanked.com/flipper/api/messages/%s/get/%s/%d/", username, messageUser, MAX_MESSAGES);
        if (!app->httpRequestAsync("messages_with_user.txt", url, GET, authHeader))
        {
            messagesStatus = MessagesRequestError;
        }
        free(url);
        free(authHeader);
        free(messageUser);
        break;
    }
    case RequestTypeMessageSend:
    {
        char *url = (char *)malloc(128);
        char *authHeader = (char *)malloc(256);
        char *message = (char *)malloc(128);
        char *messageUser = (char *)malloc(64);
        if (!url || !authHeader || !message || !messageUser)
        {
            FURI_LOG_E(TAG, "userRequest: Failed to allocate memory for url, authHeader, message or messageUser");
            messagesStatus = MessagesRequestError;
            free(username);
            free(password);
            free(payload);
            if (url)
                free(url);
            if (authHeader)
                free(authHeader);
            if (message)
                free(message);
            if (messageUser)
                free(messageUser);
            return;
        }
        // Get message from user input
        if (!app->loadChar("message_to_user", message, 128) || strlen(message) == 0 || strlen(message) > MAX_MESSAGE_LENGTH)
        {
            FURI_LOG_E(TAG, "Failed to load message to user");
            messagesStatus = MessagesRequestError;
            free(username);
            free(password);
            free(payload);
            free(url);
            free(authHeader);
            free(message);
            return;
        }
        // Get the current message user
        if (!getMessageUser(messageUser, 64))
        {
            FURI_LOG_E(TAG, "userRequest: Failed to get message user");
            messagesStatus = MessagesParseError;
            free(username);
            free(password);
            free(payload);
            free(url);
            free(authHeader);
            free(message);
            free(messageUser);
            return;
        }
        snprintf(authHeader, 256, "{\"Content-Type\":\"application/json\",\"Username\":\"%s\",\"Password\":\"%s\"}", username, password);
        snprintf(url, 128, "https://www.jblanked.com/flipper/api/messages/%s/post/", username);
        snprintf(payload, 256, "{\"receiver\":\"%s\",\"content\":\"%s\"}", messageUser, message);
        if (!app->httpRequestAsync("messages_post.txt", url, POST, authHeader, payload))
        {
            messagesStatus = MessagesRequestError;
        }
        free(url);
        free(authHeader);
        free(message);
        break;
    }
    case RequestTypeExplore:
    {
        char *url = (char *)malloc(128);
        char *authHeader = (char *)malloc(256);
        char *keyword = (char *)malloc(64);
        if (!url || !authHeader || !keyword)
        {
            FURI_LOG_E(TAG, "userRequest: Failed to allocate memory for url, authHeader or keyword");
            exploreStatus = ExploreRequestError;
            free(username);
            free(password);
            free(payload);
            if (url)
                free(url);
            if (authHeader)
                free(authHeader);
            if (keyword)
                free(keyword);
            return;
        }
        // Get keyword from user input
        if (!app->loadChar("explore_keyword", keyword, 64) || strlen(keyword) == 0 || strlen(keyword) > MAX_MESSAGE_LENGTH)
        {
            FURI_LOG_E(TAG, "Failed to load explore keyword");
            exploreStatus = ExploreRequestError;
            free(username);
            free(password);
            free(payload);
            free(url);
            free(authHeader);
            free(keyword);
            return;
        }
        snprintf(authHeader, 256, "{\"Content-Type\":\"application/json\",\"Username\":\"%s\",\"Password\":\"%s\"}", username, password);
        snprintf(url, 128, "https://www.jblanked.com/flipper/api/user/explore/%s/%d/", keyword, MAX_EXPLORE_USERS);
        if (!app->httpRequestAsync("explore.txt", url, GET, authHeader))
        {
            exploreStatus = ExploreRequestError;
        }
        free(url);
        free(authHeader);
        free(keyword);
        break;
    }
    case RequestTypePost:
    {
        char *url = (char *)malloc(128);
        char *authHeader = (char *)malloc(256);
        char *userMessage = (char *)malloc(128);
        char *feedPost = (char *)malloc(256);
        if (!url || !authHeader || !userMessage || !feedPost)
        {
            FURI_LOG_E(TAG, "userRequest: Failed to allocate memory for url, authHeader, userMessage or feedPost");
            postStatus = PostRequestError;
            free(username);
            free(password);
            if (url)
                free(url);
            if (authHeader)
                free(authHeader);
            if (userMessage)
                free(userMessage);
            if (feedPost)
                free(feedPost);
            return;
        }
        if (!app->loadChar("new_feed_post", userMessage, 128) || strlen(userMessage) == 0 || strlen(userMessage) > MAX_MESSAGE_LENGTH)
        {
            FURI_LOG_E(TAG, "Failed to load new feed post");
            postStatus = PostRequestError;
            free(username);
            free(password);
            free(url);
            free(authHeader);
            free(userMessage);
            free(feedPost);
            return;
        }
        snprintf(authHeader, 256, "{\"Content-Type\":\"application/json\",\"Username\":\"%s\",\"Password\":\"%s\"}", username, password);
        snprintf(url, 128, "https://www.jblanked.com/flipper/api/feed/post/");
        snprintf(feedPost, 256, "{\"username\":\"%s\",\"content\":\"%s\"}", username, userMessage);
        if (!app->httpRequestAsync("post.txt", url, POST, authHeader, feedPost))
        {
            postStatus = PostRequestError;
        }
        free(url);
        free(authHeader);
        free(userMessage);
        free(feedPost);
        break;
    }
    case RequestTypeCommentPost:
    {
        char *url = (char *)malloc(128);
        char *authHeader = (char *)malloc(256);
        char *userComment = (char *)malloc(128);
        char *commentPost = (char *)malloc(256);
        if (!url || !authHeader || !userComment || !commentPost)
        {
            FURI_LOG_E(TAG, "userRequest: Failed to allocate memory for url, authHeader, userComment or commentPost");
            commentsStatus = CommentsRequestError;
            free(username);
            free(password);
            if (url)
                free(url);
            if (authHeader)
                free(authHeader);
            if (userComment)
                free(userComment);
            if (commentPost)
                free(commentPost);
            return;
        }
        if (!app->loadChar("new_comment", userComment, 128) || strlen(userComment) == 0 || strlen(userComment) > MAX_MESSAGE_LENGTH)
        {
            FURI_LOG_E(TAG, "Failed to load new comment");
            commentsStatus = CommentsRequestError;
            free(username);
            free(password);
            free(url);
            free(authHeader);
            free(userComment);
            free(commentPost);
            return;
        }
        snprintf(authHeader, 256, "{\"Content-Type\":\"application/json\",\"Username\":\"%s\",\"Password\":\"%s\"}", username, password);
        snprintf(url, 128, "https://www.jblanked.com/flipper/api/feed/comment/");
        snprintf(commentPost, 256, "{\"username\":\"%s\",\"content\":\"%s\",\"post_id\":\"%u\"}", username, userComment, feedItemID);
        if (!app->httpRequestAsync("comment_post.txt", url, POST, authHeader, commentPost))
        {
            commentsStatus = CommentsRequestError;
        }
        free(url);
        free(authHeader);
        free(userComment);
        free(commentPost);
        break;
    }
    default:
        FURI_LOG_E(TAG, "Unknown request type: %d", requestType);
        loginStatus = LoginRequestError;
        registrationStatus = RegistrationRequestError;
        userInfoStatus = UserInfoRequestError;
        feedStatus = FeedRequestError;
        messageUsersStatus = MessageUsersRequestError;
        messagesStatus = MessagesRequestError;
        commentsStatus = CommentsRequestError;
        free(username);
        free(password);
        free(payload);
        return;
    }

    free(username);
    free(password);
    free(payload);
}