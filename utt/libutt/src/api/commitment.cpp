#include "commitment.hpp"
#include <utt/Comm.h>
#include <utt/Params.h>
#include <utt/RandSig.h>
#include <utt/Serialization.h>
#include <utt/PolyCrypto.h>

#include <sstream>

libutt::api::Commitment operator+(libutt::api::Commitment lhs, const libutt::api::Commitment& rhs) {
  lhs += rhs;
  return lhs;
}

std::ostream& operator<<(std::ostream& out, const libutt::api::Commitment& comm) {
  out << *(comm.comm_);
  return out;
}
std::istream& operator>>(std::istream& in, libutt::api::Commitment& comm) {
  in >> *(comm.comm_);
  return in;
}

bool operator==(const libutt::api::Commitment& comm1, const libutt::api::Commitment& comm2) {
  if (!comm1.comm_ && !comm2.comm_) return true;
  if (!comm1.comm_ || !comm2.comm_) return false;
  return *(comm1.comm_) == *(comm2.comm_);
}
namespace libutt::api {
const libutt::CommKey& Commitment::getCommitmentKey(const UTTParams& d, Commitment::Type t) {
  switch (t) {
    case Commitment::Type::REGISTRATION:
      return d.getParams().getRegCK();
    case Commitment::Type::COIN:
      return d.getParams().getCoinCK();
  }
  throw std::runtime_error("Unknown commitment key type");
}

Commitment::Commitment(const UTTParams& d, Type t, const std::vector<types::CurvePoint>& messages, bool withG2) {
  std::vector<Fr> fr_messages(messages.size());
  for (size_t i = 0; i < messages.size(); i++) {
    fr_messages[i].from_words(messages.at(i));
  }
  comm_.reset(new libutt::Comm());
  *comm_ = libutt::Comm::create(Commitment::getCommitmentKey(d, t), fr_messages, withG2);
}

Commitment::Commitment() { comm_.reset(new libutt::Comm()); }
Commitment& Commitment::operator=(const Commitment& comm) {
  *comm_ = *comm.comm_;
  return *this;
}

Commitment::Commitment(const Commitment& comm) {
  comm_.reset(new libutt::Comm());
  *this = comm;
}

Commitment& Commitment::operator+=(const Commitment& comm) {
  (*comm_) += *(comm.comm_);
  return *this;
}

types::CurvePoint Commitment::rerandomize(const UTTParams& d,
                                          Type t,
                                          std::optional<types::CurvePoint> base_randomness) {
  Fr u_delta = Fr::random_element();
  if (base_randomness.has_value()) u_delta.from_words(*base_randomness);
  comm_->rerandomize(Commitment::getCommitmentKey(d, t), u_delta);
  return u_delta.to_words();
}
}  // namespace libutt::api