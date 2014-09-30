import math
import time
import random

import wutu.events
import wutu.graphics


class Application:

    def __init__(self):
        terminus = wutu.graphics.Font()
        terminus.load('test/data/assets/fonts/terminus/ter-u12n.pcf.gz', 12)
        self.scores = Label(Vector(10, 10), terminus, 'scores: {}')
        self.scores.data = 0
        self.window = wutu.graphics.Window(640, 480, 'wutu asteroids v0.1')
        self.running = True
        self.ship = Ship(Vector(320, 240), Vector())

        self.asteroids = []

        for angle in range(45, 360, 90):

            x = math.sin(math.radians(angle)) * 200 + 320
            y = math.cos(math.radians(angle)) * 200 + 240
            asteroid = Asteroid(Vector(x, y), Vector(random.random(), random.random()).normalize() * 30, 4)
            self.asteroids.append(asteroid)

        self.bullets = []

        self.actions = {
            wutu.events.KEY_LEFT: lambda: self.ship.rotate(-5),
            wutu.events.KEY_RIGHT: lambda: self.ship.rotate(5),
            wutu.events.KEY_UP: self.ship.update_velocity,
            wutu.events.KEY_SPACE: lambda: self.bullets.append(self.ship.shoot())
        }

    def run(self):
        t = time.clock()
        while self.running:
            dt = time.clock() - t
            t = time.clock()
            self.update(dt)
            self.render()
            time.sleep(0.016)

    def update(self, dt):
        self.handle_user_input()

        bullets = []
        fragments = []
        while self.bullets:
            bullet = self.bullets.pop()
            if bullet.life_time:
                bullet.life_time -= dt
                for asteroid in self.asteroids:
                    p0 = bullet.position
                    p1 = bullet.position + bullet.velocity * dt
                    if detect_circle_line_intersection(asteroid.position, asteroid.radius, p0, p1):
                        fragments.extend(asteroid.destroy())
                        bullet.life_time = 0
                        break
                    else:
                        bullet.position = p1
                if bullet.life_time:
                    bullets.append(bullet)
        self.bullets = bullets

        asteroids = []
        while self.asteroids:
            asteroid = self.asteroids.pop()
            if not asteroid.destroyed:
                asteroids.append(asteroid)
            else:
                self.scores.data += asteroid.level
        asteroids.extend(fragments)
        self.asteroids = asteroids
        for asteroid in self.asteroids:
            asteroid.update(dt)

        self.ship.update(dt)


    def handle_user_input(self):
        for event in wutu.events.poll():
            if event.type == wutu.events.QUIT_EVENT:
                self.running = False
            if event.type == wutu.events.KEY_DOWN_EVENT:
                action = self.actions.get(event.key_code)
                if action:
                    action()

    def render(self):
        context = self.window.context
        context.clear('#252525')
        self.ship.draw(context)
        for asteroid in self.asteroids:
            asteroid.draw(context)
        for bullet in self.bullets:
            bullet.draw(context)
        self.scores.draw(context)
        self.window.update()


class Shape:

    def __init__(self, position, velocity, color='#ffffff'):
        self.position = position
        self.velocity = velocity
        self.angle = 0
        self.mesh = self.generate_mesh()
        self.color = color

    def generate_mesh(self):
        return []

    def draw(self, context):
        context.save()
        context.translate(self.position.values)
        context.rotate(self.angle)
        context.set_color(self.color)
        context.draw_line_loop(self.mesh, smooth=True)
        context.draw_line_loop(self.mesh)
        context.restore()


class Asteroid(Shape):

    def __init__(self, position, velocity, level):
        self.level = level
        self.radius = 4 << level
        self.segments = 4 + self.level * 4
        self.angle_delta = (random.random() - 0.5) * 60
        self.destroyed = False
        super().__init__(position, velocity, color='#f4b300')

    def generate_mesh(self):
        coordinates = []
        angle = 0
        while angle < 360:
            x = math.sin(math.radians(angle)) * self.radius + (random.random() - 0.5) * self.radius / 4
            y = math.cos(math.radians(angle)) * self.radius + (random.random() - 0.5) * self.radius / 4
            coordinates.append(x)
            coordinates.append(y)
            angle += 360 / self.segments
        return coordinates

    def update(self, dt):
        self.angle += dt * self.angle_delta
        self.position += self.velocity * dt
        self.position.x %= 640
        self.position.y %= 480

    def destroy(self):
        self.destroyed = True
        asteroids = []
        level = self.level - 1
        if level:
            step = random.randint(90,135)
            for angle in range(45, 360, step):
                direction = self.velocity.normalize().rotate(Vector.for_angle(angle))
                asteroid = Asteroid(self.position + direction * self.radius, direction * 30, level)
                asteroids.append(asteroid)
        return asteroids


class Ship(Shape):

    def __init__(self, position, velocity):
        self.speed = 30
        super().__init__(position, velocity, color='#78ba00')

    def generate_mesh(self):
        return (
            0, -8,
            -4, 4,
            4,  4
        )

    def rotate(self, angle):
        self.angle = (self.angle + angle) % 360

    def shoot(self):
        direction = Vector.for_angle(self.angle)
        direction.y = -direction.y
        position = self.position + direction * 10
        velocity = direction * 50
        return Bullet(position, velocity)

    def update_velocity(self):
        velocity = Vector.for_angle(self.angle) * self.speed
        velocity.y = -velocity.y
        self.velocity += velocity

    def update(self, dt):
        self.position += self.velocity * dt
        self.position.x %= 640
        self.position.y %= 480


class Bullet(Shape):

    def __init__(self, position, velocity, life_time=5.0):
        self.life_time = life_time
        super().__init__(position, velocity, color='#78ba00')

    def generate_mesh(self):
        return (
            -1.5, -1.5,
            -1.5, 1.5,
            1.5, 1.5,
            1.5, -1.5
        )


class Label:

    def __init__(self, position, font, template=''):
        self.position = position
        self.font = font
        self._text = ''
        self._data = None
        self.texture = 0
        self.image = None
        self.template = template

    @property
    def data(self):
        return self._data

    @data.setter
    def data(self, value):
        self._data = value
        self.text = self.template.format(self._data)

    @property
    def text(self):
        return self._text

    @text.setter
    def text(self, value):
        if value != self._text:
            self.image = self.font.render_text(value)
            self.texture = 0
        self._text = value

    def draw(self, context):
        if not self.texture:
            self.texture = context.create_texture(self.image)
        context.save()
        context.translate(*self.position.values)
        context.set_color('#78ba00')
        context.draw_texture(self.texture)
        context.restore()


class Vector:

    def __init__(self, x=0.0, y=0.0):
        self.x = x
        self.y = y

    def clone(self):
        return Vector(self.x, self.y)

    def __add__(self, other):
        return Vector(self.x + other.x, self.y + other.y)

    def __sub__(self, other):
        return Vector(self.x - other.x, self.y - other.y)

    def __truediv__(self, s):
        return Vector(self.x / s, self.y / s)

    def __mul__(self, s):
        return Vector(self.x * s, self.y * s)

    def dot(self, other):
        return self.x * other.x + self.y * other.y

    @property
    def length(self):
        return math.sqrt(self.x ** 2 + self.y ** 2)

    @property
    def squared_length(self):
        return self.dot(self)

    def normalize(self):
        length = self.length
        if length:
            return self / length
        else:
            return self.clone()

    def rotate(self, other):
        x = self.x * other.x - self.y * other.y
        y = self.x * other.y + self.y * other.x
        return Vector(x, y)

    @staticmethod
    def for_angle(angle, in_radians=False):
        if not in_radians:
            angle = math.radians(angle)
        return Vector(math.sin(angle), math.cos(angle))

    @property
    def values(self):
        return self.x, self.y

    def __str__(self):
        return '<Vector x={}, y={}>'.format(self.x, self.y)


def clamp01(value):
    return max(0.0, min(value, 1.0))


def detect_circle_line_intersection(center, radius, a, b):
    delta = b - a
    closest_t = clamp01(delta.dot(center - a) / delta.squared_length)
    closest = a + delta * closest_t
    return (center - closest).length <= radius


if __name__ == '__main__':
    Application().run()
