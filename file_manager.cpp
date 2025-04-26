#include "file_manager.h"
#include <LittleFS.h>

bool createFolder(const String& path) {
    if (LittleFS.exists(path)) return false;
    return LittleFS.mkdir(path);
}

bool createFile(const String& folder, const String& filename, const String& content) {
    String fullPath = folder + "/" + filename;
    File file = LittleFS.open(fullPath, FILE_WRITE);
    if (!file) return false;
    file.print(content);
    file.close();
    return true;
}

String readFile(const String& path) {
    if (!LittleFS.exists(path)) return "File not found";
    File file = LittleFS.open(path, "r");
    if (!file) return "Failed to open file";

    String content;
    while (file.available()) {
        content += (char)file.read();
    }
    file.close();
    return content;
}

bool deletePath(const String& path) {
    if (!LittleFS.exists(path)) return false;

    File entry = LittleFS.open(path);
    if (!entry) return false;

    bool isDir = entry.isDirectory();
    entry.close();

    if (isDir) {
        // Recursively delete folder contents
        File dir = LittleFS.open(path);
        File file = dir.openNextFile();
        while (file) {
            String filePath = String(file.name());
            deletePath(filePath);
            file = dir.openNextFile();
        }
        dir.close();
        return LittleFS.rmdir(path);
    } else {
        return LittleFS.remove(path);
    }
}

String listFolder(const String& path) {
    if (!LittleFS.exists(path)) return "Folder not found";

    File root = LittleFS.open(path);
    if (!root || !root.isDirectory()) return "Failed to open directory";

    String output = "[\n";
    File file = root.openNextFile();
    while (file) {
        output += "  { \"name\": \"" + String(file.name()) + "\", \"size\": " + String(file.size()) + " },\n";
        file = root.openNextFile();
    }
    if (output.endsWith(",\n")) {
        output.remove(output.length() - 2); // remove last comma
    }
    output += "\n]";
    return output;
}

void setupFileManagerEndpoints(AsyncWebServer& server) {
  server.on("/api/v1/orders/get", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!request->hasParam("path")) {
      request->send(400, "text/plain", "Missing 'path' parameter");
      return;
    }

    String filename = request->getParam("path")->value();
    if (!filename.startsWith("/")) filename = "/" + filename;

    String content = readFile(filename);
    request->send(200, "text/plain", content);
  });

  server.on("/api/v1/orders/list", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->hasParam("path")) {
      request->send(400, "text/plain", "Missing 'path' parameter");
      return;
    }

    String folder = request->getParam("path")->value();
    if (!folder.startsWith("/")) folder = "/" + folder;

    if (!LittleFS.exists(folder)) {
      request->send(404, "text/plain", "Folder not found");
      return;
    }

    File root = LittleFS.open(folder);
    if (!root || !root.isDirectory()) {
      request->send(500, "text/plain", "Failed to open directory");
      return;
    }

    String output = "[\n";
    File file = root.openNextFile();
    while (file) {
      output += "  { \"name\": \"" + String(file.name()) + "\", \"size\": " + String(file.size()) + " },\n";
      file = root.openNextFile();
    }
    if (output.endsWith(",\n")) {
      output.remove(output.length() - 2); // remove last comma
    }
    output += "\n]";

    request->send(200, "application/json", output);
  });

  server.on("/api/v1/orders/delete", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!request->hasParam("path")) {
      request->send(400, "text/plain", "Missing 'path' parameter");
      return;
    }

    String path = request->getParam("path")->value();
    if (!path.startsWith("/")) path = "/" + path;

    if (!LittleFS.exists(path)) {
      request->send(404, "text/plain", "Path does not exist");
      return;
    }

    File entry = LittleFS.open(path);
    if (!entry) {
      request->send(500, "text/plain", "Failed to open path");
      return;
    }

    bool isDir = entry.isDirectory();
    entry.close(); // must close before remove/rmdir

    bool success = false;
    if (isDir) {
      success = LittleFS.rmdir(path);
    } else {
      success = LittleFS.remove(path);
    }

    if (success) {
      request->send(200, "text/plain", "Deleted: " + path);
    } else {
      request->send(500, "text/plain", "Failed to delete: " + path);
    }
  });
}
