import config
from keras.models import *
from keras.layers import *
import keras 
import onnx
import keras2onnx

model = load_model(config.file_path+'unet_membrane.hdf5')
model.summary()
nchw_layer = Permute((3,1,2))(model.output)
nchw_model = Model(inputs = model.input, outputs = nchw_layer)
nchw_model.summary()

oml = keras2onnx.convert_keras(nchw_model, 'unet_membrane', channel_first_inputs=['input_1'],target_opset=7 )

onnx.save_model(oml, config.file_path+'unet_membrane.onnx')