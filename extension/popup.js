// popup.js

document.addEventListener('DOMContentLoaded', function() {
    document.getElementById('showData').addEventListener('click', function() {
        chrome.runtime.sendMessage({ action: 'printStorageData' }, function(response) {
            document.getElementById('content').innerText = response.data;
        });
    });

    document.getElementById('clearData').addEventListener('click', function() {
        chrome.runtime.sendMessage({ action: 'clearStorageData' }, function(response) {
            document.getElementById('content').innerText = 'Data cleared.';
        });
    });
});

// Retrieve stored content and display it in 'outputDiv'
chrome.storage.local.get({fileContent: ''}, function(result) {
    // 'result' contains the data fetched from storage
    document.getElementById('outputDiv').innerHTML = result.fileContent;
});
