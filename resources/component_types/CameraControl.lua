CameraControl = {
	OnStart = function(self)
		Camera.SetZoom(0.75)
	end,

	OnUpdate = function(self)
		Camera.SetZoom(Camera.GetZoom() + 0.005)
	end
}

