ModelRenderer = {
	modelMat = {},
	normalMat = {},
	smallMat = {},
	tinyMat = {},
	OnStart = function(self)
		for i=1,4 do
			self.smallMat[i] = {}
			self.modelMat[i] = {}
			self.tinyMat[i] = {}
			self.normalMat[i] = {}
			for j=1,4 do
				self.smallMat[i][j] = 0
				self.modelMat[i][j] = 0
				self.tinyMat[i][j] = 0
				self.normalMat[i][j] = 0
			end
		end

		self.modelMat[1][1] = 1
		self.modelMat[4][4] = 1
		self.modelMat[2][3] = -1
		self.modelMat[3][2] = -1

		self.normalMat[1][1] = 0.6
		self.normalMat[2][2] = 0.6
		self.normalMat[3][3] = 0.6
		self.normalMat[4][4] = 1

		self.smallMat[1][1] = 0.3
		self.smallMat[2][2] = 0.3
		self.smallMat[3][3] = 0.3
		self.smallMat[2][4] = 1
		self.smallMat[4][4] = 1

		self.tinyMat[1][1] = 0.2
		self.tinyMat[2][2] = 0.2
		self.tinyMat[3][3] = 0.2
		self.tinyMat[2][4] = 1
		self.tinyMat[4][4] = 1
	end,

	OnUpdate = function(self)
		
		Model.Draw("/cave/scene.gltf", "/cave/siEoZ_2K_Albedo.jpg", self.modelMat)
		-- Model.Draw("/sphere.gltf", "/default/gray.png", self.smallMat)

		-- Model.Draw("/sphere.gltf", "/default/gray.png", self.tinyMat)
		-- self.smallMat[2][4] = self.smallMat[2][4] + 0.001
		-- self.tinyMat[1][4] = self.tinyMat[1][4] + 0.001
		
		Model.Draw("/deer.gltf", "/default/gray.png", self.smallMat)
	end
}

