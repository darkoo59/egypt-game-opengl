#version 330 core

struct PositionalLight {
	vec3 Position;
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float Kc;
	float Kl;
	float Kq;
};

struct DirectionalLight {
	vec3 Position;
	vec3 Direction;
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float InnerCutOff;
	float OuterCutOff;
	float Kc;
	float Kl;
	float Kq;
};

struct Material {
	sampler2D Kd;
	sampler2D Ks;
	float Shininess;
};

uniform PositionalLight uPointLight;
uniform PositionalLight uPointLightSecond;
uniform PositionalLight uPointLightThird;
uniform DirectionalLight uSpotlight;
uniform DirectionalLight uDirLight;
uniform Material uMaterial;
uniform vec3 uViewPos;

in vec2 UV;
in vec3 vWorldSpaceFragment;
in vec3 vWorldSpaceNormal;

out vec4 FragColor;

void main() {
	vec3 ViewDirection = normalize(uViewPos - vWorldSpaceFragment);
	// NOTE(Jovan): Directional light
	vec3 DirLightVector = normalize(-uDirLight.Direction);
	float DirDiffuse = max(dot(vWorldSpaceNormal, DirLightVector), 0.0f);
	vec3 DirReflectDirection = reflect(-DirLightVector, vWorldSpaceNormal);
	float DirSpecular = pow(max(dot(ViewDirection, DirReflectDirection), 0.0f), uMaterial.Shininess);

	vec3 DirAmbientColor = uDirLight.Ka * vec3(texture(uMaterial.Kd, UV));
	vec3 DirDiffuseColor = uDirLight.Kd * DirDiffuse * vec3(texture(uMaterial.Kd, UV));
	vec3 DirSpecularColor = uDirLight.Ks * DirSpecular * vec3(texture(uMaterial.Ks, UV));
	vec3 DirColor = DirAmbientColor + DirDiffuseColor + DirSpecularColor;

	// Point light
	vec3 PtLightVector = normalize(uPointLight.Position - vWorldSpaceFragment);
	float PtDiffuse = max(dot(vWorldSpaceNormal, PtLightVector), 0.0f);
	vec3 PtReflectDirection = reflect(-PtLightVector, vWorldSpaceNormal);
	float PtSpecular = pow(max(dot(ViewDirection, PtReflectDirection), 0.0f), uMaterial.Shininess);

	vec3 PtAmbientColor = uPointLight.Ka * vec3(texture(uMaterial.Kd, UV));
	vec3 PtDiffuseColor = PtDiffuse * uPointLight.Kd * vec3(texture(uMaterial.Kd, UV));
	vec3 PtSpecularColor = PtSpecular * uPointLight.Ks * vec3(texture(uMaterial.Ks, UV));

	float PtLightDistance = length(uPointLight.Position - vWorldSpaceFragment);
	float PtAttenuation = 1.0f / (uPointLight.Kc + uPointLight.Kl * PtLightDistance + uPointLight.Kq * (PtLightDistance * PtLightDistance));
	vec3 PtColor = PtAttenuation * (PtAmbientColor + PtDiffuseColor + PtSpecularColor);

	// Point light second
	PtLightVector = normalize(uPointLightSecond.Position - vWorldSpaceFragment);
	PtDiffuse = max(dot(vWorldSpaceNormal, PtLightVector), 0.0f);
	PtReflectDirection = reflect(-PtLightVector, vWorldSpaceNormal);
	PtSpecular = pow(max(dot(ViewDirection, PtReflectDirection), 0.0f), uMaterial.Shininess);

	PtAmbientColor = uPointLightSecond.Ka * vec3(texture(uMaterial.Kd, UV));
	PtDiffuseColor = PtDiffuse * uPointLightSecond.Kd * vec3(texture(uMaterial.Kd, UV));
	PtSpecularColor = PtSpecular * uPointLightSecond.Ks * vec3(texture(uMaterial.Ks, UV));

	PtLightDistance = length(uPointLightSecond.Position - vWorldSpaceFragment);
	PtAttenuation = 1.0f / (uPointLightSecond.Kc + uPointLightSecond.Kl * PtLightDistance + uPointLightSecond.Kq * (PtLightDistance * PtLightDistance));
	PtColor += PtAttenuation * (PtAmbientColor + PtDiffuseColor + PtSpecularColor);

	// Point light third
	PtLightVector = normalize(uPointLightThird.Position - vWorldSpaceFragment);
	PtDiffuse = max(dot(vWorldSpaceNormal, PtLightVector), 0.0f);
	PtReflectDirection = reflect(-PtLightVector, vWorldSpaceNormal);
	PtSpecular = pow(max(dot(ViewDirection, PtReflectDirection), 0.0f), uMaterial.Shininess);

	PtAmbientColor = uPointLightThird.Ka * vec3(texture(uMaterial.Kd, UV));
	PtDiffuseColor = PtDiffuse * uPointLightThird.Kd * vec3(texture(uMaterial.Kd, UV));
	PtSpecularColor = PtSpecular * uPointLightThird.Ks * vec3(texture(uMaterial.Ks, UV));

	PtLightDistance = length(uPointLightThird.Position - vWorldSpaceFragment);
	PtAttenuation = 1.0f / (uPointLightThird.Kc + uPointLightThird.Kl * PtLightDistance + uPointLightThird.Kq * (PtLightDistance * PtLightDistance));
	PtColor += PtAttenuation * (PtAmbientColor + PtDiffuseColor + PtSpecularColor);

	// Spotlight
	vec3 SpotlightVector = normalize(uSpotlight.Position - vWorldSpaceFragment);

	float SpotDiffuse = max(dot(vWorldSpaceNormal, SpotlightVector), 0.0f);
	vec3 SpotReflectDirection = reflect(-SpotlightVector, vWorldSpaceNormal);
	float SpotSpecular = pow(max(dot(ViewDirection, SpotReflectDirection), 0.0f), uMaterial.Shininess);

	vec3 SpotAmbientColor = uSpotlight.Ka * vec3(texture(uMaterial.Kd, UV));
	vec3 SpotDiffuseColor = SpotDiffuse * uSpotlight.Kd * vec3(texture(uMaterial.Kd, UV));
	vec3 SpotSpecularColor = SpotSpecular * uSpotlight.Ks * vec3(texture(uMaterial.Ks, UV));

	float SpotlightDistance = length(uSpotlight.Position - vWorldSpaceFragment);
	float SpotAttenuation = 1.0f / (uSpotlight.Kc + uSpotlight.Kl * SpotlightDistance + uSpotlight.Kq * (SpotlightDistance * SpotlightDistance));

	float Theta = dot(SpotlightVector, normalize(-uSpotlight.Direction));
	float Epsilon = uSpotlight.InnerCutOff - uSpotlight.OuterCutOff;
	float SpotIntensity = clamp((Theta - uSpotlight.OuterCutOff) / Epsilon, 0.0f, 1.0f);
	vec3 SpotColor = SpotIntensity * SpotAttenuation * (SpotAmbientColor + SpotDiffuseColor + SpotSpecularColor);
	
	vec3 FinalColor = DirColor + PtColor + SpotColor;
	FragColor = vec4(FinalColor, 1.0f);
}
