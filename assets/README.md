# FlipSocial
The first social media app for Flipper Zero. Connect with other users directly on your device through WiFi.

The highlight of this app is customizable pre-saves, which, as explained below, aim to address the challenges of typing with the directional pad.

FlipSocial uses the FlipperHTTP flash for the WiFi Devboard, first introduced in the WebCrawler app: https://github.com/jblanked/WebCrawler-FlipperZero/tree/main/assets/FlipperHTTP

## Requirements
- WiFi Developer Board, Raspberry Pi, or ESP32 Device with FlipperHTTP Flash: https://github.com/jblanked/FlipperHTTP
- WiFi Access Point


## Features
- Login/Logout
- Registration
- Feed
- Profile
- Customizable Pre-Saves
- Explore (NEW)
- Friends (NEW)
- Direct Messaging (NEW)

**Login/Logout:** Log in to your account to view and post on the Feed. You can also change your password and log out when needed.

**Registration:** Create an account with just a username and password—no email or personal information required or collected.

**Feed:** View up to 50 of the latest posts, create your own posts, and "Flip" a post—FlipSocial’s version of liking or favoriting a post.

**Customizable Pre-Saves:** The biggest challenge with a social media app on the Flipper Zero is using only the directional pad for input. To address this, I implemented a pre-saved text system. The pre-saves are stored in a pre_saved_messages.txt file on your SD card. You can edit the pre-saves by opening qFlipper, downloading the file from the /apps_data/flip_social/ folder, adding your pre-saves (separated by new lines), and then copying it back to your SD card. You can also create pre-saves directly within the app.

**Explore:** Discover other users and add them as friends.

**Friends:** View and remove friends.

**Direct Messaging:** Send direct messages to other Flipper users and view your conversations.

## Roadmap
**v0.2**
- Stability Patch

**v0.3**
- Explore Page
- Friends

**v0.4**
- Direct Messaging

**v0.5**
- Improve memory allocation
- Improve Feed Page
- Raspberry Pi Pico W Support

**v0.6**
- Improve memory allocation
- Update the Direct Messaging View
- Update the Pre-Save View

**v0.7**
- Improve memory allocation
- Loading screens.

**v0.8**
- Improve User Profile
- Improve Explore Page

**v1.0**
- Official Release