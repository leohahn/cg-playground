#include "camera.hpp"
#include "lt_utils.hpp"

static lt::Logger logger("camera");

void
update_frustum_right_and_up(Frustum& frustum, Vec3f up_world)
{
    Vec3f right_vec = lt::normalize(lt::cross(frustum.front.v, up_world));
    Vec3f up_vec = lt::normalize(lt::cross(right_vec, frustum.front.v));

    frustum.right = Quatf(0, right_vec);
    frustum.up = Quatf(0, up_vec);
}

Camera::Camera(Vec3f position, Vec3f front_vec, Vec3f up_world,
               f32 fovy, f32 ratio, f32 move_speed, f32 rotation_speed)
    : up_world(up_world)
    , move_speed(move_speed)
    , rotation_speed(rotation_speed)
{
    logger.log("initializing frustum");
    frustum.front = Quatf(0, lt::normalize(front_vec));
    frustum.position = position;
    frustum.ratio = ratio;
    frustum.fovy = fovy;
    frustum.znear = ZNEAR;
    frustum.zfar = ZFAR;
    frustum.projection = lt::perspective(lt::radians(fovy), ratio, ZNEAR, ZFAR);

    update_frustum_right_and_up(frustum, up_world);
}

void
Camera::move(Direction dir, f64 delta)
{
    f32 offset = move_speed * delta;
    switch (dir)
    {
    case Direction::Left: {
        frustum.position -= frustum.right.v * offset;
    } break;

    case Direction::Right: {
        frustum.position += frustum.right.v * offset;
    } break;

    case Direction::Forwards: {
        frustum.position += frustum.front.v * offset;
    } break;

    case Direction::Backwards: {
        frustum.position -= frustum.front.v * offset;
    } break;

    default:
        LT_Assert(false);
    }
}

void
Camera::rotate(RotationAxis axis, f64 delta)
{
    switch (axis)
    {
    case RotationAxis::Up: {
        Quatf rotated_front = lt::rotate(frustum.front, (f32)delta*rotation_speed, frustum.up);
        frustum.front = rotated_front;
    } break;

    case RotationAxis::Right: {
        Quatf rotated_front = lt::rotate(frustum.front, (f32)delta*rotation_speed, frustum.right);
        frustum.front = rotated_front;
    } break;

    default:
        LT_Assert(false);
    }

    update_frustum_right_and_up(frustum, up_world);
}

Mat4f
Camera::view_matrix() const
{
    return lt::look_at(frustum.position, frustum.position + frustum.front.v, frustum.up.v);
}
