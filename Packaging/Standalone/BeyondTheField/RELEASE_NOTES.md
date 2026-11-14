# **Update 2 Hotfix**

**More Reliable Startup**
- Fixed a startup issue that could leave Social Media or recruiting/fog inactive when Windows was still updating the game's loaded-file list.
- Fixed the same check reporting a startup failure even when the runtime had already loaded successfully.
- Retries are brief and limited to checking that list. The plugin does not repeat an uncertain runtime load.

**Clearer Error Details**
- Startup logs now name the affected BTF component, the step that failed and the Windows error code.
- Settings, Recruit Notes, accounts and published Social Media history are kept.

**Install**
- Close MMC and the game, then replace **BeyondTheField.dll** in your usual **Plugins** folder. COT users can explicitly check for updates and choose **1.0.25** for BTF only.
- The private build completed a clean live startup with both runtimes verified and Social Media ready. Broader intermittent-startup testing remains ongoing.
- The separate **"Could not find part of the path"** reports are still under investigation pending logs; this hotfix does not claim to resolve them.
