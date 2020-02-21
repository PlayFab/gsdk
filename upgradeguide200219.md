---
title: Upgrade guide for 0.7.190918 -> 0.7.200220
author: gufabre
description: Description of breaking changes of new cpp nuget release.
ms.author: gufabre
ms.date: 02/19/2020
ms.topic: article
ms.prod: playfab
keywords: playfab, development, release, apis, features
ms.localizationpriority: medium
---

# Upgrade guide for 0.7.190918 -> 0.7.200220

The 0.7.200220 release of the GSDK for C++ contains breaking changes. The primary change in this update is provide thread safety. Previous implementations allowed a thread to alter a dictionary while another thread read it which caused errors and raised exceptions. This topic provides detailed instructions on the steps you must take to update your code when you update to current release.

The PR contains a detailed explanation of the changes: [c++ change for thread safety #43](https://github.com/PlayFab/gsdk/pull/43/)

Most customers are only affected by 3 very minor breaking changes in this release. Nearly all customers will need to change 1 or 2 lines of code, but very few customers might need to change more. The upgrade guide below cover all of the possible changes required for even the most obscure breaking changes.

## Upgrade instructions - Affects most/all customers


The following methods have been updated to return an object instead of a reference:
1. `GSDK::getConfigSettings()`
2. `GSDK::getLogsDirectory()`
3. `GSDK::getSharedContentDirectory()`

* You must update your code as follows to accept the returned object:

    `GSDK::getConfigSettings()`
    * Old: ```const std::unordered_map<std::string, std::string>& config = GSDK::getConfigSettings();```
    * New: ```const std::unordered_map<std::string, std::string> config = GSDK::getConfigSettings();```

    `GSDK::getLogsDirectory()`
    * Old: ```const std::string& logsDirectory = GSDK::getLogsDirectory();```
    * New: ```const std::string logsDirectory = GSDK::getLogsDirectory();```

    `GSDK::getSharedContentDirectory()`
    * Old: ```const std::string& sharedContentDirectory = GSDK::getSharedContentDirectory();```
    * New: ```const std::string sharedContentDirectory = GSDK::getSharedContentDirectory();```

We apologize that while trivial, these changes were necessary in order to ensure thread safety.
