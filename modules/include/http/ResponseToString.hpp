#pragma once

#include "ziapi/Http.hpp"

static const inline std::map<ziapi::http::Code, std::string> kStatusCodeMap{
    {ziapi::http::Code(100), ziapi::http::reason::kContinue},
    {ziapi::http::Code(101), ziapi::http::reason::kSwitchingProtocols},
    {ziapi::http::Code(200), ziapi::http::reason::kOK},
    {ziapi::http::Code(201), ziapi::http::reason::kCreated},
    {ziapi::http::Code(202), ziapi::http::reason::kAccepted},
    {ziapi::http::Code(203), ziapi::http::reason::kNonAuthoritativeInformation},
    {ziapi::http::Code(204), ziapi::http::reason::kNoContent},
    {ziapi::http::Code(205), ziapi::http::reason::kResetContent},
    {ziapi::http::Code(206), ziapi::http::reason::kPartialContent},
    {ziapi::http::Code(300), ziapi::http::reason::kMultipleChoices},
    {ziapi::http::Code(301), ziapi::http::reason::kMovedPermanently},
    {ziapi::http::Code(302), ziapi::http::reason::kFound},
    {ziapi::http::Code(303), ziapi::http::reason::kSeeOther},
    {ziapi::http::Code(304), ziapi::http::reason::kNotModified},
    {ziapi::http::Code(305), ziapi::http::reason::kUseProxy},
    {ziapi::http::Code(307), ziapi::http::reason::kTemporaryRedirect},
    {ziapi::http::Code(400), ziapi::http::reason::kBadRequest},
    {ziapi::http::Code(401), ziapi::http::reason::kUnauthorized},
    {ziapi::http::Code(402), ziapi::http::reason::kPaymentRequired},
    {ziapi::http::Code(403), ziapi::http::reason::kForbidden},
    {ziapi::http::Code(404), ziapi::http::reason::kNotFound},
    {ziapi::http::Code(405), ziapi::http::reason::kMethodNotAllowed},
    {ziapi::http::Code(406), ziapi::http::reason::kNotAcceptable},
    {ziapi::http::Code(407), ziapi::http::reason::kProxyAuthenticationRequired},
    {ziapi::http::Code(408), "Request Timeout"},
    {ziapi::http::Code(409), ziapi::http::reason::kConflicT},  // ziapi typo
    {ziapi::http::Code(410), ziapi::http::reason::kGone},
    {ziapi::http::Code(411), ziapi::http::reason::kLengthRequired},
    {ziapi::http::Code(412), ziapi::http::reason::kPreconditionFailed},
    {ziapi::http::Code(413), ziapi::http::reason::kRequestEntityTooLarge},
    {ziapi::http::Code(414), ziapi::http::reason::kRequestURITooLarge},
    {ziapi::http::Code(415), ziapi::http::reason::kUnsupportedMediaType},
    {ziapi::http::Code(416), ziapi::http::reason::kRequestedRangeNotSatisfiable},
    {ziapi::http::Code(417), ziapi::http::reason::kExpectationFailed},
    {ziapi::http::Code(500), ziapi::http::reason::kInternalServerError},
    {ziapi::http::Code(501), ziapi::http::reason::kNotImplemented},
    {ziapi::http::Code(502), ziapi::http::reason::kBadGateway},
    {ziapi::http::Code(503), ziapi::http::reason::kServiceUnavailable},
    {ziapi::http::Code(504), ziapi::http::reason::kGatewayTimeout},
    {ziapi::http::Code(505), ziapi::http::reason::kHttpVersionNotSupported},
};

std::string ResponseToString(const ziapi::http::Response &r);