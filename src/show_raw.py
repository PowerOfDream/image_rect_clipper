from PIL import Image

def show()：
    f = open("out.raw", "rb")
    data = f.read(512*512)
    img = Image.frombytes('L', (512, 512), data)
    img.show()


