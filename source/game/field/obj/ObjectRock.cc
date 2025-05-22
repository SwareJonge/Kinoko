#include "ObjectRock.hh"

#include "game/field/CollisionDirector.hh"

#include "game/kart/KartCollide.hh"

namespace Field {

/// @addr{0x8076F344} 
ObjectRock::~ObjectRock() = default;

/// @addr{0x8076F384}
void ObjectRock::init() {
    m_railInterpolator->init(0.0f, 0);
    m_railInterpolator->setCurrVel(static_cast<f32>(m_mapObj->setting(2)));
    m_railInterpolator->calc();
    
    m_state = State::Tangible;

    m_startYPos = m_railInterpolator->curPos().y;
    m_flags |= 1; // POSITION_DIRTY
    m_pos.y = m_startYPos;

    EGG::Vector3f curTanDirNorm = m_railInterpolator->curTangentDir();
    curTanDirNorm.normalise();
    setMatrixTangentTo(EGG::Vector3f::ey, curTanDirNorm);
    calcTransform();
    m_angRad = 0.0f;
    m_angSpd = INITIAL_ANGULAR_SPEED;
    m_cooldownTimer = m_mapObj->setting(0);
    m_colTranslate.x = 0.0f;
    m_colTranslate.y = static_cast<f32>(m_mapObj->setting(3));
    m_colTranslate.z = 0.0f;
    calcTransform();
}

/// @addr{0x8076F590}
void ObjectRock::calc() {
    switch (m_state)
    {
    case State::Tangible:
        calcTangible();
        break;
    case State::Intangible:
        calcIntangible();
        break;
    }

    --m_cooldownTimer;
    EGG::Vector3f scaledTang = m_railInterpolator->curTangentDir() * m_railInterpolator->getCurrVel();
    m_colTranslate.x = scaledTang.x;
    m_colTranslate.z = scaledTang.z;
}

// @addr{0x8076F768}
void ObjectRock::calcTangible() {
    auto railStatus = m_railInterpolator->calc();
    if (railStatus == RailInterpolator::Status::ChangingDirection) {
        breakRock(EGG::Vector3f::ey);
    }

    m_flags |= 1; // POSITION_DIRTY
    m_pos = m_railInterpolator->curPos();
    m_colTranslate.y = m_colTranslate.y - 2.0f;

    m_flags |= 1; // POSITION_DIRTY
    m_pos += m_colTranslate;

    checkSphereFull();
    calcTangibleSub();
}

// @addr {0x8076FA60}
void ObjectRock::checkSphereFull() {
    CollisionInfo info;
    info.bbox.setZero();
    
    EGG::Vector3f offset(0.0f, m_scale.x * 240.0f - 50.0f, 0.0f);
    EGG::Vector3f pos = m_pos + offset;

    if (CollisionDirector::Instance()->checkSphereFull(50.0f, pos, EGG::Vector3f::inf,
                KCL_TYPE_FLOOR, &info, nullptr, 0)) {
        m_colTranslate.y *= -0.3f;
        m_angSpd = std::max((static_cast<f32>(m_mapObj->setting(2)) * 360.0f) / (m_scale.x * 480.0f * F_PI), m_angSpd + 1.0f);
        m_flags |= 1; // POSITION_DIRTY
        m_pos += info.tangentOff;
    }
}

/// @addr{0x8076FD90}
void ObjectRock::breakRock(const EGG::Vector3f &hitDepth) {
    EGG::Vector3f up;
    up.y = 0.0f;
    if (hitDepth.x != 0.0f && hitDepth.z != 0.0f) {
        up.x = -hitDepth.z;
        up.z = hitDepth.x;
    } else {
        up.x = 1.0f;
        up.z = 0.0f;
    }

    EGG::Matrix34f mat;
    SetRotTangentHorizontal(mat, up, hitDepth);

    m_state = State::Intangible;
    m_railInterpolator->init(0.0f, 0);
    m_railInterpolator->setCurrVel(static_cast<f32>(m_mapObj->setting(2)));
    
    m_startYPos = m_railInterpolator->curPos().y;
    m_flags |= 1; // POSITION_DIRTY
    m_pos.y = m_startYPos;
    //initModels(0);
    disableCollision();
}

/// @addr{0x8076F868}
void ObjectRock::calcIntangible() {
    if (m_cooldownTimer < 0) {
        m_state = State::Tangible;
        m_angSpd = INITIAL_ANGULAR_SPEED;
        m_colTranslate.y = static_cast<f32>(m_mapObj->setting(3));
        m_cooldownTimer = m_mapObj->setting(1);
        //initModels(1);
        enableCollision();
    }
}

// @addr{0x80770068}
Kart::Reaction ObjectRock::onCollision(Kart::KartObject * /*kartObj*/,
Kart::Reaction reactionOnKart, Kart::Reaction reactionOnObj,
EGG::Vector3f &hitDepth) {
    switch (reactionOnObj) {
        case Kart::Reaction::None: {
            if (m_scale.x < 1.0f) {
                breakRock(hitDepth);
            }
            reactionOnKart = Kart::Reaction::None;
            break;
        }
        default:
            break;
    }
    return reactionOnKart;
}

/// @addr{0x8076F91C} 
void ObjectRock::calcTangibleSub() {
    EGG::Vector3f tangDir = m_railInterpolator->curTangentDir();
    tangDir.y = 0.0f;
    tangDir.normalise();
    EGG::Matrix34f m = FUN_806B3CA4(tangDir);
    m_angRad += m_angSpd * DEG2RAD;

    EGG::Matrix34f mat(EGG::Matrix34f::ident);
    EGG::Vector3f vRot(m_angRad, 0.0f, 0.0f);
    mat.makeR(vRot);
    m_flags |= 4; // MATRIX_DIRTY
    m_transform = m.multiplyTo(mat);
    m_transform.setBase(3, m_pos);
}

}