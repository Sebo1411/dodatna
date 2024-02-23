// background.js

// Object to store tab titles
var tabTitles = {};

// Function to append tab titles to a file
function appendToFile(tabTitle) {
    // Get current date and time
    var now = new Date();
    var dateTime = now.toLocaleDateString() + ' ' + now.toLocaleTimeString();

    // Append date and time to tab title
    tabTitle = dateTime + ': ' + tabTitle;

    // Write tab title to a text file using Chrome FileSystem API
    chrome.storage.local.get({fileContent: ''}, function(data) {
        // Append tab title to existing file content
        var updatedContent = data.fileContent + tabTitle + '<br>';

        // Write updated content back to storage
        chrome.storage.local.set({fileContent: updatedContent}, function() {
            console.log('Tab title appended to file: ' + tabTitle);
        });
    });
}

// Event listener for when a tab is created
chrome.tabs.onCreated.addListener(function(tab) {
    // Store the tab title
    tabTitles[tab.id] = tab.title;
    appendToFile('Tab Created: ' + tab.title);
});

// Event listener for when a tab is updated
chrome.tabs.onUpdated.addListener(function(tabId, changeInfo, tab) {
    // Update the tab title
    tabTitles[tab.id] = tab.title;
});

// Event listener for when a tab is removed (closed)
chrome.tabs.onRemoved.addListener(function(tabId, removeInfo) {
    // Get the title of the closed tab from our stored data
    var tabTitle = tabTitles[tabId];
    if (tabTitle) {
        appendToFile('Tab Removed: ' + tabTitle);
        // Remove the title from our stored data
        delete tabTitles[tabId];
    }
});

// Function to retrieve and print all data from chrome.storage.local
function printStorageData() {
    chrome.storage.local.get(null, function(data) {
        console.log('Data stored in chrome.storage.local:', data);
    });
}

// Function to clear all data from chrome.storage.local
function clearStorageData() {
    chrome.storage.local.clear(function() {
        console.log('Data cleared from chrome.storage.local');
    });
}

// Listen for messages from the popup
chrome.runtime.onMessage.addListener(function(request, sender, sendResponse) {
    if (request.action === 'printStorageData') {
        chrome.storage.local.get(null, function(data) {
            console.log('Data stored in chrome.storage.local:', data);
            sendResponse({ data: JSON.stringify(data, null, 2) });
        });
    } else if (request.action === 'clearStorageData') {
        chrome.storage.local.clear(function() {
            console.log('Data cleared from chrome.storage.local');
            sendResponse({ data: 'Data cleared.' });
        });
    }

    // Indicate that the response function will be called asynchronously
    return true;
});