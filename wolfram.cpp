/* КОМАНДА ДЛЯ КОМПИЛЯЦИИ clang++ wolfram.cpp -o WolframAlphaProgram -lcurl
 * НЕОБХОДИМЫЕ БИБЛИОТЕКИ 1) libcurl (sudo apt-get install libcur14-openssl-dev)
 *                        2) nlohmann/json (sudo apt install nlohmann-json3-dev)
 *                                                 /|\
 *                                                  |
 *                                                  |
 *                                           БЕЗ -get
 */
#include <curl/curl.h>

#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

size_t WriteCallback(void* contents, size_t size, size_t nmemb,
                     std::string* buffer) {
  size_t totalSize = size * nmemb;
  buffer->append((char*)contents, totalSize);
  return totalSize;
}
int main() {
  std::cout << "Программа на основе API WolframAlpha,состоящая из <90 строк "
               "кода, созданная, чтобы \n"
               "насмехаться над теми, кто реально искал мат. формулы и тратил "
               "десятки часов своего времени на то,\nчтобы решить уравнение 4 "
               "степени или "
               "написать свой косинус\nВыйти --- exit\n\n";
  const std::string appID = "YXJ36V-WKW2PP2H72";
  while (true) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    std::vector<std::string> results;
    std::cout << "Ваш ввод: ";
    std::string query;
    std::getline(std::cin, query);
    if (query == "exit") {
      break;
    }
    curl = curl_easy_init();
    if (curl) {
      char* encodedQuery =
          curl_easy_escape(curl, query.c_str(), query.length());
      std::string url = "http://api.wolframalpha.com/v2/query?input=" +
                        std::string(encodedQuery) +
                        "&format=plaintext&output=JSON&appid=" + appID;
      curl_free(encodedQuery);
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
      curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
      res = curl_easy_perform(curl);
      if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res)
                  << std::endl;
      } else {
        try {
          json jsonResponse = json::parse(readBuffer);
          if (jsonResponse.contains("queryresult") &&
              jsonResponse["queryresult"]["success"].get<bool>()) {
            auto pods = jsonResponse["queryresult"]["pods"];
            bool resultFound = false;
            for (const auto& pod : pods) {
              std::string title = pod["title"].get<std::string>();

              if (title == "Decimal approximation" || title == "Result" ||
                  title == "Solutions" || title == "Real solutions" ||
                  title == "Complex solutions") {
                for (const auto& subpod : pod["subpods"]) {
                  results.push_back(subpod["plaintext"].get<std::string>());
                }
                resultFound = true;
              }
            }
            if (!resultFound) {
              std::cerr << "Корни или результат не найдены." << std::endl;
            }
          } else {
            std::cerr << "Ошибка: Запрос не удался или Wolfram Alpha не вернул "
                         "результат."
                      << std::endl;
          }
        } catch (json::parse_error& e) {
          std::cerr << "Ошибка парсинга JSON: " << e.what() << std::endl;
        }
      }
      curl_easy_cleanup(curl);
    }
    if (!results.empty()) {
      std::cout << "Результаты:\n";
      for (const std::string& res : results) {
        std::cout << res << std::endl;
      }
    } else {
      std::cerr << "Результат не получен." << std::endl;
    }
  }
  return 0;
}
