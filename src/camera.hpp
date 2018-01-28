#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "lt_math.hpp"

struct Frustum
{
    Vec3f  position;
    f32    ratio;
    f32    fovy;
    f32    znear;
    f32    zfar;

    Mat4f  projection;
    // Calculated attributes
    Quatf  front;
    Quatf  right;
    Quatf  up;
};

struct Camera
{
    static constexpr f32 ZNEAR = 0.1f;
    static constexpr f32 ZFAR = 1000.0f;

    enum class Direction { Left, Right, Forwards, Backwards };
    enum class RotationAxis { Up, Right };

    Frustum frustum;
    f32     yaw;
    f32     pitch;
    Vec3f   up_world;
    f32     move_speed;
    f32     rotation_speed;

    Camera(Vec3f position, Vec3f front_vec, Vec3f up_world,
           f32 fovy, f32 ratio, f32 move_speed, f32 rotation_speed);

    void move(Direction dir);
    void rotate_positive(RotationAxis axis);
    void rotate_negative(RotationAxis axis);
    Mat4f view_matrix() const;
};

#endif // CAMERA_HPP
