# FlipSocial
The first social media app for Flipper Zero. Connect with other users directly on your device through WiFi. The highlight of this app is customizable pre-saves, which, as explained below, aim to solve the dissatisfaction of typing with the directional pad. FlipSocial uses the FlipperHTTP flash for the WiFi Devboard, first introduced in the WebCrawler app: https://github.com/jblanked/WebCrawler-FlipperZero/tree/main/assets/FlipperHTTP

## Features
- Login/Logout
- Registration
- Feed
- Profile
- Customizable Pre-Saves
- Direct Messaging (coming soon)

**Login/Logout**
Log in to your account to view and post on the Feed. You can also change your password and log out when needed.

**Registration**
Create an account with just a username and password—no email or personal information required or collected.

**Feed**
View up to 128 of the latest posts, create your own posts, and "Flip" a post—FlipSocial’s version of liking or favoriting a post.

**Customizable Pre-Saves**
The biggest challenge with a social media app on the Flipper Zero is using only the directional pad for input. As a solution, I implemented a pre-saved text system. The pre-saves are stored in a pre_saved_messages.txt file on your SD card. You can edit the pre-saves by opening qFlipper, downloading the file from the /apps_data/flip_social/ folder, adding your pre-saves (separated by new lines), and then copying it back to your SD card. You can also create the pre-saves directly within the app.

## Roadmap
**v0.2**
- Stability Patch
- LED options

**v0.3**
- Explore Page
- Friends

**v0.4**
- Direct Messaging
- Privacy Settings

## Contribution
This is a big task, and I welcome all contributors, especially developers who are into animations and graphics. Fork the repository, create a pull request, and I will review your edits.

## Known Bugs
1. When clicking any button other than the BACK button in the Feed view or the post creation view, the app doesn't respond to inputs.
   - Solution: Restart your Flipper device.
   
2. When trying to log in, the app shows "Awaiting response..." and nothing happens for more than 30 seconds.
   - Solution 1: Update your WiFi credentials. Sometimes you just need to hit Save again on either the SSID or password.
   - Solution 2: Ensure your WiFi Devboard is plugged in, then restart the app.
   - Solution 3: Ensure your WiFi Devboard is plugged in, then restart your Flipper device.
   
3. When accessing the Feed, I keep getting the message "Either the feed didn’t load or there was a server error."
   - Solution 1: Update your WiFi credentials. Sometimes you just need to hit Save again on either the SSID or password.
   - Solution 2: Ensure your WiFi Devboard is plugged in, then restart the app.
   - Solution 3: Ensure your WiFi Devboard is plugged in, then restart your Flipper device.
   
4. The Feed is empty.
   - Solution 1: Update your WiFi credentials. Sometimes you just need to hit Save again on either the SSID or password.
   - Solution 2: Ensure your WiFi Devboard is plugged in, then restart the app.
   - Solution 3: Ensure your WiFi Devboard is plugged in, then restart your Flipper device.