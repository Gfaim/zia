#include "Notes.hpp"

#include "dylib/dylib.hpp"

void NotesModule::Init(const ziapi::config::Node &cfg) { folder_ = cfg["modules"]["notes"]["folder"].AsString(); }

std::string NotesModule::NoteToJson(int id, const Note &note)
{
    return "{\"id\":" + std::to_string(id) + "\"title\":\"" + note.title + "\",\"description\":\"" + note.description +
           "\"}";
}

int NotesModule::GetIdFromTarget(const std::string &target)
{
    auto tmp = target.find("id=");
    auto id = 0;
    if (tmp != std::string::npos) {
        return std::atoi(&target[tmp + 3]);
    }
    return -1;
}

NotesModule::Note NotesModule::GetNoteFromBody(const std::string &body)
{
    auto tmp = body.find('\n');
    Note res;

    res.title = body.substr(0, tmp);
    res.description = body.substr(tmp + 1);
    return res;
}

void NotesModule::Get(int id, ziapi::http::Response &res) const
{
    res.status_code = ziapi::http::Code::kOK;
    if (id == -1) {
        res.body = "[";
        for (auto &note : notes_) {
            res.body += NoteToJson(note.first, note.second) + ",";
        }
        if (res.body.size() == 1) {
            res.body += ']';
        }
        res.body[res.body.size() - 1] = ']';
        return;
    }
    res.body = NoteToJson(id, notes_.at(id));
}

void NotesModule::Post(const std::string &title, const std::string &description, ziapi::http::Response &res)
{
    notes_[notes_.size()] = Note{title, description};
    res.status_code = ziapi::http::Code::kOK;
}

void NotesModule::Put(int id, const std::string &title, const std::string &description, ziapi::http::Response &res)
{
    if (id == -1) {
        res.status_code = ziapi::http::Code::kBadRequest;
        return;
    }
    notes_[id].title = title;
    notes_[id].description = description;
    res.status_code = ziapi::http::Code::kOK;
}

void NotesModule::Delete(int id, ziapi::http::Response &res)
{
    if (id == -1) {
        res.status_code = ziapi::http::Code::kBadRequest;
        return;
    }
    notes_.erase(notes_.find(id));
    res.status_code = ziapi::http::Code::kOK;
}

#include <iostream>
void NotesModule::Handle(ziapi::http::Context &ctx, const ziapi::http::Request &req, ziapi::http::Response &res)
{
    std::cout << "Notes handler" << std::endl;
    auto id = GetIdFromTarget(req.target);
    auto body = GetNoteFromBody(req.body);

    if (id != -1 and notes_.find(id) == notes_.end()) {
        res.status_code = ziapi::http::Code::kNotFound;
        return;
    }
    {
        std::scoped_lock lock(mu_);
        
        if (req.method == "GET") {
            return Get(id, res);
        } else if (req.method == "DELETE") {
            return Delete(id, res);
        } else if (req.method == "POST") {
            return Post(body.title, body.description, res);
        } else if (req.method == "PUT") {
            return Put(id, body.title, body.description, res);
        } else {
            res.status_code = ziapi::http::Code::kNotFound;
        }
    }
}

DYLIB_API ziapi::IModule *LoadZiaModule() { return new NotesModule(); }
