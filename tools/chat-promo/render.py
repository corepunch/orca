#!/usr/bin/env python3
"""Compose Metro-inspired artwork from real, offline iPhone screenshots."""
import hashlib
import json
from pathlib import Path
import zipfile
from PIL import Image, ImageDraw, ImageFont, ImageOps

ROOT = Path(__file__).resolve().parents[2]
OUTPUT = ROOT / 'marketing/chat/app-store'
TYPEFACE = '/System/Library/Fonts/HelveticaNeue.ttc'
SIZES = [(1320, 2868), (1206, 2622)]
PAPER, INK = '#faf9f6', '#111111'


def font(size, weight='light'):
    return ImageFont.truetype(TYPEFACE, size, index={'regular': 0, 'bold': 1, 'light': 7}[weight])


def capture(shot):
    path = OUTPUT / shot['capture']
    if hashlib.sha256(path.read_bytes()).hexdigest() != shot['sha256']:
        raise ValueError('Capture changed since manifest: ' + str(path))
    return Image.open(path).convert('RGB')


def phone(canvas, screen, x, y, width, outline=12):
    """Simple iPhone outline around the complete, unretouched capture."""
    height = round(screen.height * width / screen.width)
    draw = ImageDraw.Draw(canvas)
    radius = round(width * .106)
    for inset, color in [(outline+4, '#262626'), (outline+1, '#b7b7b7'), (outline-2, '#545454'), (5, '#080808')]:
        draw.rounded_rectangle((x-inset, y-inset, x+width+inset, y+height+inset),
                               radius=radius+inset, fill=color)
    mask = Image.new('L', (width, height))
    ImageDraw.Draw(mask).rounded_rectangle((0, 0, width-1, height-1), radius=radius, fill=255)
    canvas.paste(screen.resize((width, height), Image.Resampling.LANCZOS), (x, y), mask)
    return height


def contact_panorama(images):
    """Unfold two native viewports using their actual 342pt pan and 48pt overlap.

    Re-typeset the shared heading only in the exploded marketing view. Framed
    phones always display complete simulator captures.
    """
    first, second = images['contacts'], images['contacts-more']
    assert first.size == second.size == (1170, 2532)
    strip = Image.new('RGB', (2196, 2532), 'black')
    strip.paste(first, (0, 0))
    strip.paste(second.crop((144, 0, 1170, 2532)), (1170, 0))
    draw = ImageDraw.Draw(strip)
    draw.rectangle((0, 0, 2196, 722), fill='black')
    draw.rectangle((0, 2250, 2196, 2532), fill='black')
    draw.text((30, 172), 'contacts', font=font(340), fill='white')
    draw.text((36, 565), 'recent', font=font(96), fill='white')
    return strip


def render(shot, index, images, wall):
    width, height = SIZES[0]
    canvas = Image.new('RGB', (width, height), PAPER)
    draw = ImageDraw.Draw(canvas)
    draw.text((88, 88), 'CHAT', font=font(34, 'bold'), fill=INK)
    draw.text((1232, 91), 'iPHONE', anchor='ra', font=font(28, 'regular'), fill='#555555')
    size = 116
    while max(draw.textlength(line, font=font(size)) for line in shot['title'].splitlines()) > 1144:
        size -= 1
    if size < 90:
        raise ValueError('Headline too small: ' + shot['title'])
    for line_index, line in enumerate(shot['title'].splitlines()):
        draw.text((82, 218+line_index*128), line, font=font(size), fill=INK)
    draw.text((90, 520), shot['subtitle'], font=font(34, 'regular'), fill='#555555')
    if draw.textlength(shot['subtitle'], font=font(34, 'regular')) > 1140:
        raise ValueError('Subtitle overflows')
    # The app surface extends beyond its outlined phone, as in the references.
    x, y, screen_width = 202, 682, 916
    draw.rectangle((0, 845, width, 2500), fill='black')
    if shot['stage'] in ('contacts', 'contacts-more'):
        scale = screen_width / 1170
        viewport_offset = 0 if shot['stage'] == 'contacts' else 1026
        unfolded = wall.crop((0, 180, wall.width, 2250))
        unfolded = unfolded.resize((round(unfolded.width*scale), round(unfolded.height*scale)), Image.Resampling.LANCZOS)
        canvas.paste(unfolded, (x-round(viewport_offset*scale), y+round(180*scale)))
    else:
        order = ['contacts-all', 'inbox', 'direct', 'photos', 'group', 'channel']
        position = order.index(shot['stage']) if shot['stage'] in order else 1
        for direction in (-1, 1):
            neighbor = images[order[(position+direction) % len(order)]]
            crop = neighbor.crop((0, 180, neighbor.width, 2250))
            scale = screen_width / neighbor.width
            crop = crop.resize((screen_width, round(crop.height*scale)), Image.Resampling.LANCZOS)
            canvas.paste(crop, (x+direction*(screen_width+34), y+round(180*scale)))
    phone(canvas, images[shot['stage'] if shot['appearance'] != 'light' else 'light'], x, y, screen_width)
    draw = ImageDraw.Draw(canvas)
    draw.line((88, 2768, 1232, 2768), fill='#cececa', width=2)
    draw.text((88, 2794), 'A LITTLE CLOSER.', font=font(24, 'regular'), fill='#505050')
    draw.text((1232, 2794), f'{index+1:02} / 08', anchor='ra', font=font(24, 'regular'), fill='#505050')
    return canvas


def render_wide(images, wall):
    canvas = Image.new('RGB', (4096, 1740), PAPER)
    draw = ImageDraw.Draw(canvas)
    draw.text((80, 66), 'CHAT / iPHONE', font=font(38, 'bold'), fill=INK)
    draw.text((72, 145), 'a little closer.', font=font(162), fill=INK)
    draw.text((82, 346), 'Your people. Your conversations. A little closer.', font=font(43), fill='#555555')
    # Contact wall spans two viewports, then All and four more actual screens.
    pieces = [wall] + [images[stage] for stage in ['contacts-all', 'inbox', 'direct', 'photos', 'group']]
    scale = 3936 / sum(part.width for part in pieces)
    top, x = 405, 80
    for part in pieces:
        content = part.crop((0, 180, part.width, 2250))
        size = (round(part.width*scale), round(content.height*scale))
        canvas.paste(content.resize(size, Image.Resampling.LANCZOS), (round(x), top+round(180*scale)))
        x += part.width*scale
    # A complete iPhone capture anchors the unfolded app to its actual device.
    all_x = round(80+wall.width*scale)
    phone(canvas, images['contacts-all'], all_x, top, round(1170*scale), outline=9)
    draw = ImageDraw.Draw(canvas)
    draw.line((80, 1690, 4016, 1690), fill='#cccccc', width=2)
    labels = [(80, 'contacts'), (all_x, 'all')]
    labels += [(round(80+(wall.width+i*1170)*scale), label)
               for i, label in enumerate(['chats', 'messages', 'photos', 'groups'], 1)]
    for x, label in labels:
        draw.text((x, 1700), label, font=font(25), fill='#555555')
    canvas.save(OUTPUT / 'panorama-wide.png', optimize=True)
    canvas.resize((2048, 870), Image.Resampling.LANCZOS).save(OUTPUT / 'panorama-preview.jpg', quality=94)
    return canvas


def render_creatives(images, wall, panorama):
    """Dedicated Apple header and search placements, exported without alpha."""
    directory = OUTPUT / 'creatives'
    directory.mkdir(parents=True, exist_ok=True)
    header = ImageOps.pad(panorama, (3840, 1646), method=Image.Resampling.LANCZOS, color=PAPER)
    header.save(directory / 'header-3840x1646.png', optimize=True)
    search = Image.new('RGB', (3840, 2560), PAPER)
    draw = ImageDraw.Draw(search)
    draw.text((150, 80), 'CHAT / iPHONE', font=font(43, 'bold'), fill=INK)
    draw.text((136, 174), 'a little closer.', font=font(225), fill=INK)
    draw.text((154, 472), 'Your people. Your conversations.', font=font(65), fill='#555555')
    draw.rectangle((0, 770, 3840, 2275), fill='black')
    scale, top = 850 / 1170, 630
    x = round(530-1026*scale)
    for part in [wall, images['contacts-all'], images['inbox'], images['direct']]:
        content = part.crop((0, 180, part.width, 2250))
        size = (round(part.width*scale), round(content.height*scale))
        search.paste(content.resize(size, Image.Resampling.LANCZOS), (x, top+round(180*scale)))
        x += size[0]+30
    phone(search, images['contacts-more'], 530, top, 850)
    phone(search, images['inbox'], 2440, top, 850)
    search.save(directory / 'search-results-3840x2560.png', optimize=True)
    records = []
    for name in ('header-3840x1646.png', 'search-results-3840x2560.png'):
        path = directory / name
        with Image.open(path) as check:
            assert check.mode == 'RGB'
            width, height = check.size
        records.append(dict(path=str(path.relative_to(OUTPUT)), width=width, height=height,
                            sha256=hashlib.sha256(path.read_bytes()).hexdigest()))
    return records


def main():
    manifest = json.loads((OUTPUT / 'manifest.json').read_text())
    images = {('light' if shot['appearance'] == 'light' else shot['stage']): capture(shot)
              for shot in manifest['shots']}
    wall = contact_panorama(images)
    shots = [shot for shot in manifest['shots'] if not shot.get('auxiliary')]
    assert len(shots) == 8, 'Expected eight App Store screenshots'
    rendered, files = [], []
    for i, shot in enumerate(shots):
        artwork = render(shot, i, images, wall)
        rendered.append(artwork)
        for width, height in SIZES:
            directory = OUTPUT / f'en-US/iphone-{width}x{height}'
            directory.mkdir(parents=True, exist_ok=True)
            target = directory / (shot['name']+'.png')
            export = artwork if artwork.size == (width, height) else artwork.resize((width, height), Image.Resampling.LANCZOS)
            export.save(target, optimize=True)
            with Image.open(target) as check:
                assert check.mode == 'RGB' and check.size == (width, height)
            files.append(dict(path=str(target.relative_to(OUTPUT)), width=width, height=height,
                              sha256=hashlib.sha256(target.read_bytes()).hexdigest()))
    thumb_w, thumb_h, gap = 330, 717, 20
    sheet = Image.new('RGB', (thumb_w*4+gap*5, thumb_h*2+gap*3), '#deded9')
    for i, artwork in enumerate(rendered):
        sheet.paste(artwork.resize((thumb_w, thumb_h), Image.Resampling.LANCZOS),
                    (gap+i%4*(thumb_w+gap), gap+i//4*(thumb_h+gap)))
    sheet.save(OUTPUT / 'contact-sheet.jpg', quality=95)
    panorama = render_wide(images, wall)
    manifest['creative_exports'] = render_creatives(images, wall, panorama)
    manifest['exports'] = files
    manifest['art_direction'] = 'Unfolded Metro panorama with real iPhone captures and synthetic candid fixtures.'
    (OUTPUT / 'manifest.json').write_text(json.dumps(manifest, indent=2)+'\n')
    with zipfile.ZipFile(OUTPUT / 'chat-iphone-app-store-en-US.zip', 'w', zipfile.ZIP_DEFLATED) as archive:
        for file in files:
            archive.write(OUTPUT / file['path'], file['path'])
    with zipfile.ZipFile(OUTPUT / 'chat-iphone-app-store-en-US.zip') as archive:
        assert set(archive.namelist()) == {file['path'] for file in files}
        assert archive.testzip() is None
    print(f'Verified {len(files)} RGB PNGs; exported panorama, contact sheet and upload ZIP.')


if __name__ == '__main__':
    main()
