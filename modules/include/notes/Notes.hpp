#pragma once

#include <ziapi/Module.hpp>
#include <mutex>

class NotesModule : public ziapi::IHandlerModule {
private:
    struct Note {
        std::string title;
        std::string description;
    };

    std::mutex mu_{};

    std::string folder_;
    std::map<int, Note> notes_;

    static std::string NoteToJson(int id, const Note &note);
    static int GetIdFromTarget(const std::string &target);
    static Note GetNoteFromBody(const std::string &body);

    void Get(int id, ziapi::http::Response &res) const;
    void Post(const std::string &title, const std::string &description, ziapi::http::Response &res);
    void Put(int id, const std::string &title, const std::string &description, ziapi::http::Response &res);
    void Delete(int id, ziapi::http::Response &res);

public:
    void Init(const ziapi::config::Node &cfg) override;

    [[nodiscard]] ziapi::Version GetVersion() const noexcept override { return {5, 0, 1}; }

    [[nodiscard]] ziapi::Version GetCompatibleApiVersion() const noexcept override { return {5, 0, 1}; }

    [[nodiscard]] const char *GetName() const noexcept override { return "NotesModule"; }

    [[nodiscard]] const char *GetDescription() const noexcept override { return "CRUD operations on note objects"; }

    [[nodiscard]] double GetHandlerPriority() const noexcept override { return 0; }

    [[nodiscard]] bool ShouldHandle(const ziapi::http::Context &ctx, const ziapi::http::Request &req) const override
    {
        return req.target.rfind("/notes", 0) == 0;
    }

    void Handle(ziapi::http::Context &ctx, const ziapi::http::Request &req, ziapi::http::Response &res) override;
};
